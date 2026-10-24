// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

use crate::utils::Logger;
use dioxus::prelude::*;
use std::{
    io::{BufRead, BufReader, Read},
    process::Stdio,
};
use tokio::sync::mpsc::{unbounded_channel, UnboundedSender};

fn forward_lines(stream: impl Read + Send + 'static, tx: UnboundedSender<String>) {
    std::thread::spawn(move || {
        let mut reader = BufReader::new(stream);
        let mut buffer = Vec::new();
        loop {
            buffer.clear();
            match reader.read_until(b'\n', &mut buffer) {
                Ok(0) | Err(_) => break,
                Ok(_) => {}
            }
            if tx.send(String::from_utf8_lossy(&buffer).into_owned()).is_err() {
                break;
            }
        }
    });
}

#[derive(Clone, PartialEq, Debug)]
pub struct MachOItem {
    pub id: usize,
    pub title: String,
    pub path: String,
}

#[derive(Clone, Copy)]
pub struct MachOListState {
    pub items: Signal<Vec<MachOItem>>,
    pub startup_argument: Signal<String>,
}

impl MachOListState {
    pub fn new() -> Self {
        Self {
            items: Signal::new(vec![]),
            startup_argument: Signal::new(String::new()),
        }
    }

    pub fn add(&mut self, title: String, path: String) {
        let mut items = self.items.write();
        let id = items.iter().map(|item| item.id).max().unwrap_or(0) + 1;
        items.push(MachOItem { id, title, path });
    }
}

#[component]
pub fn MachOList() -> Element {
    let mut logger = use_context::<Logger>();
    let mut list = use_context::<MachOListState>();

    rsx! {
        div { id: "macholist",
            div { id: "machos", class: "flex flex-col gap-2",
                for item in list.items.read().iter().cloned() {
                    MachOItemRow {
                        key: "{item.id}",
                        item: item.clone(),
                        on_delete: move |deleted_item: MachOItem| {
                            let deleted_id = deleted_item.id;
                            list.items.write().retain(|item| item.id != deleted_id);
                            logger.append(&format!("Deleted Mach-O target: {}", deleted_item.title));
                        },
                    }
                }
            }
        }
    }
}

#[component]
fn MachOItemRow(item: MachOItem, on_delete: EventHandler<MachOItem>) -> Element {
    let mut logger = use_context::<Logger>();
    let mut info_logger = use_context::<Logger>();
    let list = use_context::<MachOListState>();
    let start_item = item.clone();
    let info_item = item.clone();

    rsx! {
        div { class: "macho-item flex items-center justify-between gap-2 rounded-md bg-slate-800/60 px-3 py-2",
            span {
                class: "min-w-0 cursor-pointer truncate text-sm text-slate-200",
                title: "{item.path}",
                onclick: move |_| {
                    info_logger
                        .append(
                            &format!(
                                "Mach-O target: {} (ID: {}, path: {})",
                                info_item.title,
                                info_item.id,
                                info_item.path,
                            ),
                        )
                },
                "{item.title}"
            }
            div { class: "flex shrink-0 gap-1",
                button {
                    class: "macho-action macho-start rounded px-2 py-1 text-xs font-medium",
                    onclick: move |_| {
                        let argument = list.startup_argument.read().clone();
                        // Re-launch this executable in CLI mode: MachOEngine <macho> [arguments...]
                        let exe = match std::env::current_exe() {
                            Ok(exe) => exe,
                            Err(error) => {
                                logger.append(&format!("Failed to locate MachOEngine: {error}"));
                                return;
                            }
                        };
                        let mut command = std::process::Command::new(exe);
                        command
                            .arg(&start_item.path)
                            .args(argument.split_whitespace())
                            .stdin(Stdio::null())
                            .stdout(Stdio::piped())
                            .stderr(Stdio::piped());
                        match command.spawn() {
                            Ok(mut child) => {
                                let (tx, mut rx) = unbounded_channel::<String>();
                                if let Some(out) = child.stdout.take() {
                                    forward_lines(out, tx.clone());
                                }
                                if let Some(err) = child.stderr.take() {
                                    forward_lines(err, tx.clone());
                                }
                                let pid = child.id();
                                let title = start_item.title.clone();
                                // Reaper thread reports the exit status once both pipes are drained.
                                std::thread::spawn(move || {
                                    let status = child.wait();
                                    let _ = tx.send(match status {
                                        Ok(status) => format!("Mach-O target {title} exited: {status}"),
                                        Err(error) => format!("Failed to wait for {title}: {error}"),
                                    });
                                });
                                spawn(async move {
                                    while let Some(line) = rx.recv().await {
                                        logger.append(line.trim_end_matches(['\n', '\r']));
                                    }
                                });
                                logger
                                    .append(
                                        &format!(
                                            "Started Mach-O target {}{} (PID {})",
                                            start_item.title,
                                            if argument.is_empty() {
                                                String::new()
                                            } else {
                                                format!(" with argument {argument:?}")
                                            },
                                            pid,
                                        ),
                                    )
                            }
                            Err(error) => {
                                logger
                                    .append(
                                        &format!(
                                            "Failed to start Mach-O target {}: {error}",
                                            start_item.title,
                                        ),
                                    )
                            }
                        }
                    },
                    "Start"
                }
                button {
                    class: "macho-action macho-delete rounded px-2 py-1 text-xs font-medium",
                    onclick: move |_| on_delete.call(item.clone()),
                    "Delete"
                }
            }
        }
    }
}

use crate::utils::Logger;
use dioxus::prelude::*;

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
                        let mut command = std::process::Command::new(&start_item.path);
                        if !argument.is_empty() {
                            command.arg(&argument);
                        }
                        match command.spawn() {
                            Ok(child) => {
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
                                            child.id(),
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

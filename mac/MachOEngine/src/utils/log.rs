// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

use dioxus::prelude::*;
use std::{
    fs::File,
    io::{BufRead, BufReader},
    os::fd::{FromRawFd, RawFd},
    sync::OnceLock,
};
use tokio::sync::mpsc::{unbounded_channel, UnboundedReceiver, UnboundedSender};

static STDIO_CAPTURED: OnceLock<()> = OnceLock::new();

/// Redirects process-wide stdout/stderr (including output from native code) into a
/// channel of lines. Output is still forwarded to the original descriptors.
/// Returns `None` if already captured or if the redirection fails.
pub fn capture_stdio() -> Option<UnboundedReceiver<String>> {
    STDIO_CAPTURED.set(()).ok()?;
    let (tx, rx) = unbounded_channel();
    for fd in [libc::STDOUT_FILENO, libc::STDERR_FILENO] {
        redirect_fd(fd, tx.clone())?;
    }
    Some(rx)
}

fn redirect_fd(target: RawFd, tx: UnboundedSender<String>) -> Option<()> {
    let mut pipe = [0 as RawFd; 2];
    unsafe {
        if libc::pipe(pipe.as_mut_ptr()) != 0 {
            return None;
        }
        let original = libc::dup(target);
        if original < 0 || libc::dup2(pipe[1], target) < 0 {
            libc::close(pipe[0]);
            libc::close(pipe[1]);
            return None;
        }
        libc::close(pipe[1]);

        let reader = BufReader::new(File::from_raw_fd(pipe[0]));
        std::thread::spawn(move || forward_lines(reader, original, tx));
    }
    Some(())
}

fn forward_lines(mut reader: BufReader<File>, original: RawFd, tx: UnboundedSender<String>) {
    let mut buffer = Vec::new();
    loop {
        buffer.clear();
        match reader.read_until(b'\n', &mut buffer) {
            Ok(0) | Err(_) => break,
            Ok(_) => {}
        }
        unsafe {
            libc::write(original, buffer.as_ptr().cast(), buffer.len());
        }
        let line = String::from_utf8_lossy(&buffer);
        let line = line.trim_end_matches(['\n', '\r']);
        if tx.send(line.to_string()).is_err() {
            break;
        }
    }
}

// 1. Define the struct that holds the shared state (signal)
#[derive(Clone, Copy, Debug)]
pub struct Logger {
    inner: Signal<String>,
}

impl Logger {
    // 2. Encapsulate the initialization logic
    pub fn new() -> Self {
        Self {
            inner: use_signal(|| String::new()),
        }
    }

    // 3. Encapsulate the modification logic and 'pub' export it
    pub fn append(&mut self, msg: &str) {
        let timestamp = chrono::Local::now().format("%H:%M:%S");
        // Access and update the inner signal directly
        self.inner.push_str(&format!("[{timestamp}] {}\n", msg));
    }

    // A helper for the view component to read data easily
    pub fn get_messages(&self) -> String {
        self.inner.read().clone()
    }

    pub fn clear(&mut self) {
        self.inner.set(String::new());
    }
}

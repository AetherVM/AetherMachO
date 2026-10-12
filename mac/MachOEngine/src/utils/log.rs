// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

use chrono::prelude::*;
use dioxus::prelude::*;

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

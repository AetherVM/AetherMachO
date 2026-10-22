// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

mod engine;
mod log;

pub use engine::{engine_main, EngineMainError};
pub use log::{capture_stdio, Logger};

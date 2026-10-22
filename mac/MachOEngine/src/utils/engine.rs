// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

use std::{
    ffi::{c_char, c_int, CString, NulError, OsString},
    os::unix::ffi::OsStrExt,
};

#[derive(Debug)]
pub enum EngineMainError {
    ArgumentContainsNul(NulError),
    TooManyArguments,
}

impl std::fmt::Display for EngineMainError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::ArgumentContainsNul(error) => {
                write!(f, "engine argument contains an interior NUL byte: {error}")
            }
            Self::TooManyArguments => write!(f, "too many arguments for engine_main"),
        }
    }
}

impl std::error::Error for EngineMainError {}

#[link(name = "MachOEngine")]
unsafe extern "C" {
    #[link_name = "engine_main"]
    fn engine_main_ffi(
        argc: c_int,
        argv: *const *const c_char,
        vmargc: c_int,
        vmargv: *const *const c_char,
    ) -> c_int;
}

/// Calls the C++ entry point; `argv` includes the program name as its first item.
pub fn engine_main(argv: &[OsString], vm_argv: &[OsString]) -> Result<c_int, EngineMainError> {
    let argv = to_c_strings(argv)?;
    let vm_argv = to_c_strings(vm_argv)?;
    let argc = c_int::try_from(argv.len()).map_err(|_| EngineMainError::TooManyArguments)?;
    let vmargc = c_int::try_from(vm_argv.len()).map_err(|_| EngineMainError::TooManyArguments)?;
    let argv = argv.iter().map(|arg| arg.as_ptr()).collect::<Vec<_>>();
    let vm_argv = vm_argv.iter().map(|arg| arg.as_ptr()).collect::<Vec<_>>();

    // The C++ entry point consumes the argument pointers only during this call.
    Ok(unsafe { engine_main_ffi(argc, argv.as_ptr(), vmargc, vm_argv.as_ptr()) })
}

fn to_c_strings(args: &[OsString]) -> Result<Vec<CString>, EngineMainError> {
    args.iter()
        .map(|arg| {
            CString::new(arg.as_os_str().as_bytes()).map_err(EngineMainError::ArgumentContainsNul)
        })
        .collect()
}

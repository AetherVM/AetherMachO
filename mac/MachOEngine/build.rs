// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

use std::{env, path::PathBuf};

fn main() {
    println!("cargo:rustc-env=DIOXUS_APP_TITLE=MachOEngine");
    println!("cargo:rustc-env=DIOXUS_PRODUCT_NAME=MachOEngine");

    println!("cargo:rerun-if-env-changed=AETHER_MACHO_ENGINE_LIB_DIR");
    let manifest_dir = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let repo_root = manifest_dir.join("../..");
    let lib_dir = env::var_os("AETHER_MACHO_ENGINE_LIB_DIR")
        .map(PathBuf::from)
        .or_else(|| {
            [
                repo_root.join("build-mac-Debug"),
                repo_root.join("build-mac-Release"),
                repo_root.join("build-mac-RelWithDebInfo"),
                repo_root.join("build-mac-Debug/install/lib"),
                repo_root.join("build-mac-Release/install/lib"),
            ]
            .into_iter()
            .find(|path| path.join("libMachOEngine.dylib").is_file())
        })
        .filter(|path| path.join("libMachOEngine.dylib").is_file())
        .unwrap_or_else(|| {
            panic!(
                "could not locate libMachOEngine.dylib; set AETHER_MACHO_ENGINE_LIB_DIR to its directory"
            )
        });
    let lib_dir = lib_dir.canonicalize().unwrap_or(lib_dir);

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=dylib=MachOEngine");
    println!("cargo:rustc-link-arg=-Wl,-rpath,{}", lib_dir.display());
}

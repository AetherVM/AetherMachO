// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

// The dioxus prelude contains a ton of common items used in dioxus apps. It's a good idea to import wherever you need dioxus
use dioxus::prelude::*;

use components::{Log, MachOList, MachOListState};
use std::{
    ffi::OsString,
    io::Read,
    path::{Path, PathBuf},
};
use utils::{engine_main, Logger};

/// Define a components module that contains all shared components for our app.
mod components;
pub mod utils;

// We can import assets in dioxus with the `asset!` macro. This macro takes a path to an asset relative to the crate root.
// The macro returns an `Asset` type that will display as the path to the asset in the browser or a local path in desktop bundles.
// The asset macro also minifies some assets like CSS and JS to make bundled smaller
const MAIN_CSS: Asset = asset!("/assets/styling/main.css");
const TAILWIND_CSS: Asset = asset!("/assets/tailwind.css");

fn main() {
    const APP_NAME: &str = "MachOEngine";

    // Dioxus reads these values when building the desktop window title and the macOS About panel.
    std::env::set_var("DIOXUS_APP_TITLE", APP_NAME);
    std::env::set_var("DIOXUS_PRODUCT_NAME", APP_NAME);

    // The `launch` function is the main entry point for a dioxus app. It takes a component and renders it with the platform feature you have enabled
    dioxus_native::launch(App);
}

/// App is the main component of our app. Components are the building blocks of dioxus apps. Each component is a function
/// that takes some props and returns an Element. In this case, App takes no props because it is the root of our app.
///
/// Components should be annotated with `#[component]` to support props, better error messages, and autocomplete
#[component]
fn App() -> Element {
    let mut logger = Logger::new();
    use_context_provider(|| logger);
    use_context_provider(MachOListState::new);
    let mut macho_list = use_context::<MachOListState>();

    rsx! {
        document::Link { rel: "stylesheet", href: MAIN_CSS }
        document::Link { rel: "stylesheet", href: TAILWIND_CSS }

        // Root Window Canvas
        div {
            id: "Container",
            class: "flex flex-row w-screen h-screen bg-slate-950 text-slate-100 p-4 gap-4 overflow-hidden select-none font-sans vscode-theme",

            // Left Sidebar Panel
            div {
                id: "LeftContainer",
                class: "flex flex-col w-64 h-full gap-3 shrink-0 vscode-sidebar",

                // Header Label
                div {
                    id: "macho-heading",
                    class: "px-2 py-1 text-xs font-semibold text-slate-400 uppercase tracking-wider",
                    "Mach-O Executables"
                }

                // Auto-Scrolling Sidebar List
                div {
                    id: "MachOListContainer",
                    class: "flex-1 overflow-y-auto min-h-0 bg-slate-900/60 border border-slate-800 rounded-xl p-2 shadow-inner vscode-list",
                    MachOList {}
                }

                label {
                    id: "startup-argument-label",
                    r#for: "startup-argument",
                    class: "text-xs font-medium text-slate-400",
                    "Startup argument"
                }
                input {
                    id: "startup-argument",
                    r#type: "text",
                    class: "w-full rounded-lg border border-slate-700 bg-slate-900 px-3 py-2 text-sm text-slate-100 placeholder-slate-500 focus:border-blue-500 focus:outline-none",
                    placeholder: "Optional argument",
                    value: "{macho_list.startup_argument.read()}",
                    oninput: move |event| macho_list.startup_argument.set(event.value()),
                }

                // Action Button
                button {
                    id: "import-macho",
                    class: "w-full py-2.5 px-4 bg-blue-600 hover:bg-blue-500 active:bg-blue-700 text-white font-medium text-sm rounded-lg shadow-md transition-all duration-150 shrink-0 flex items-center justify-center gap-2",
                    onclick: move |_| {
                        if let Some(path) = rfd::FileDialog::new().pick_file_or_folder() {
                            match resolve_macho(&path) {
                                Ok((title, executable)) => {
                                    macho_list
                                        .add(title.clone(), executable.to_string_lossy().into_owned());
                                    logger.append(&format!("Imported Mach-O target: {title}"));
                                    let mut path_argument = OsString::from("-path=");
                                    path_argument.push(executable.as_os_str());
                                    let engine_args = [
                                        OsString::from("MachOEngine"),
                                        OsString::from("-action=encode"),
                                        path_argument,
                                    ];
                                    match engine_main(&engine_args, &[]) {
                                        Ok(0) => logger
                                            .append(&format!("Encoded Mach-O target: {title}")),
                                        Ok(status) => logger.append(&format!(
                                            "Failed to encode Mach-O target {title}: engine_main returned {status}"
                                        )),
                                        Err(error) => logger.append(&format!(
                                            "Failed to encode Mach-O target {title}: {error}"
                                        )),
                                    }
                                }
                                Err(error) => {
                                    logger
                                        .append(&format!("Failed to import {}: {error}", path.display()))
                                }
                            }
                        }
                    },
                    "📥 Import MachO"
                }
            }

            // Right Main Console / Log View
            div {
                id: "log-panel",
                class: "flex-1 h-full min-h-0 flex flex-col bg-slate-900/60 border border-slate-800 rounded-xl p-3 shadow-inner overflow-hidden",
                div {
                    id: "log-heading",
                    class: "px-2 pb-2 text-xs font-semibold text-slate-400 uppercase tracking-wider border-b border-slate-800/80 mb-2 flex justify-between items-center",
                    span { "Execution & Analysis Log" }
                    span { class: "text-[10px] text-slate-500 font-mono", "TTY 1" }
                }
                Log {}
            }
        }
    }
}

fn resolve_macho(path: &Path) -> Result<(String, PathBuf), String> {
    if path
        .extension()
        .is_some_and(|extension| extension.eq_ignore_ascii_case("app"))
    {
        let macos_info_plist = path.join("Contents/Info.plist");
        let info_plist = if macos_info_plist.is_file() {
            macos_info_plist
        } else {
            path.join("Info.plist")
        };
        let plist = plist::Value::from_file(&info_plist)
            .map_err(|error| format!("could not read Info.plist: {error}"))?;
        let executable_name = plist
            .as_dictionary()
            .and_then(|dictionary| dictionary.get("CFBundleExecutable"))
            .and_then(plist::Value::as_string)
            .ok_or_else(|| "Info.plist has no CFBundleExecutable".to_string())?;
        let executable = resolve_bundle_executable(path, executable_name)?;
        validate_macho(&executable)?;
        let title = path
            .file_stem()
            .and_then(|name| name.to_str())
            .unwrap_or(executable_name)
            .to_string();
        Ok((title, executable))
    } else if path.is_file() {
        validate_macho(path)?;
        let title = path
            .file_name()
            .and_then(|name| name.to_str())
            .ok_or_else(|| "selected file has no valid name".to_string())?
            .to_string();
        Ok((title, path.to_path_buf()))
    } else {
        Err("select a Mach-O file or an .app bundle".to_string())
    }
}

fn resolve_bundle_executable(bundle_path: &Path, executable_name: &str) -> Result<PathBuf, String> {
    let executable_name_path = Path::new(executable_name);
    if executable_name.is_empty()
        || executable_name_path.components().count() != 1
        || executable_name_path.file_name().is_none()
    {
        return Err("Info.plist has an invalid CFBundleExecutable".to_string());
    }

    let bundle_root = bundle_path
        .canonicalize()
        .map_err(|error| format!("could not resolve app bundle path: {error}"))?;

    for candidate in [
        bundle_path.join("Contents/MacOS").join(executable_name),
        bundle_path.join(executable_name),
    ] {
        let Ok(executable) = candidate.canonicalize() else {
            continue;
        };
        if executable.is_file() && executable.starts_with(&bundle_root) {
            return Ok(executable);
        }
    }

    Err(format!(
        "bundle executable {executable_name:?} was not found in Contents/MacOS or the app bundle root"
    ))
}

fn validate_macho(path: &Path) -> Result<(), String> {
    let mut file =
        std::fs::File::open(path).map_err(|error| format!("could not open executable: {error}"))?;
    let mut magic = [0; 4];
    file.read_exact(&mut magic)
        .map_err(|error| format!("could not read executable header: {error}"))?;

    match magic {
        [0xfe, 0xed, 0xfa, 0xce]
        | [0xce, 0xfa, 0xed, 0xfe]
        | [0xfe, 0xed, 0xfa, 0xcf]
        | [0xcf, 0xfa, 0xed, 0xfe]
        | [0xca, 0xfe, 0xba, 0xbe]
        | [0xbe, 0xba, 0xfe, 0xca]
        | [0xca, 0xfe, 0xba, 0xbf]
        | [0xbf, 0xba, 0xfe, 0xca] => Ok(()),
        _ => Err("file does not have a valid Mach-O header".to_string()),
    }
}

use crate::utils::Logger;
use dioxus::prelude::*;

#[component]
pub fn Log() -> Element {
    let mut logger = use_context::<Logger>();
    let messages = logger.get_messages();

    rsx! {
      div {
        id: "log",
        class: "flex-1 flex flex-col h-full w-full min-h-0 overflow-hidden vscode-log",

        div { class: "flex justify-end gap-2 pb-2 log-actions",
          button {
            class: "px-3 py-1 text-xs font-medium text-slate-300 bg-slate-800 hover:bg-slate-700 rounded-md",
            onclick: move |_| logger.clear(),
            "Clear"
          }
          button {
            class: "px-3 py-1 text-xs font-medium text-slate-300 bg-slate-800 hover:bg-slate-700 rounded-md",
            onclick: move |_| {
                let contents = logger.get_messages();
                if let Some(path) = rfd::FileDialog::new()
                    .add_filter("Text files", &["txt"])
                    .set_file_name("MachOEngine.txt")
                    .save_file()
                {
                    if let Err(error) = std::fs::write(&path, contents) {
                        logger
                            .append(
                                &format!("Failed to export log to {}: {error}", path.display()),
                            );
                    }
                }
            },
            "Export"
          }
        }

        div {
          id: "log_output",
          class: "flex-1 h-full w-full p-3 flex flex-col-reverse font-mono text-xs text-emerald-400 bg-slate-950 border border-slate-800/60 rounded-lg focus:outline-none leading-relaxed overflow-y-auto selection:bg-slate-800",
          for (index , line) in messages.lines().rev().enumerate() {
            div {
              key: "{index}",
              class: "w-full shrink-0 whitespace-pre-wrap break-words",
              "{line}"
            }
          }
        }
      }
    }
}

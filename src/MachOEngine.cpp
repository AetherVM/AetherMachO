// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#include <MachOEngine.h>
#include <Utils.h>

namespace aether {

MachOEngine::MachOEngine(std::string_view path, EventConfig eventcfg)
    : BinaryEngine(&m_machine, eventcfg), m_path(path) {}

MachOEngine::~MachOEngine() {}

bool MachOEngine::preEncode() { return false; }

addr_t MachOEngine::recursiveLoad() { return 0; }

} // namespace aether

int engine_main(int argc, const char *argv[], int vmargc,
                const char *vmargv[]) {
  int dbgport = 61012; // defined on 12/Oct/2026
  if (argc == 1) {
    aether::log_print(aether::Runtime,
                      "Usage: MachOEngine -action=<encode|run> "
                      "-path=</path/to/macho> [-debug] [-dbgport=<{}>]",
                      dbgport);
    return 0;
  }

  bool debug = false;
  std::string_view action, path;
  for (int i = 1; i < argc; ++i) {
    std::string_view arg{argv[i]};
    if (arg.starts_with("-action="))
      action = arg.substr(8);
    else if (arg.starts_with("-path="))
      path = arg.substr(6);
    else if (arg.starts_with("-debug"))
      debug = true;
    else if (arg.starts_with("-dbgport="))
      dbgport = std::stoi(arg.substr(9).data());
  }
  if (!fs::exists(path)) {
    aether::log_print(aether::Runtime, "The path '{}' doesn't exist.", path);
    return -1;
  }

  aether::EventConfig eventcfg;
  if (debug) {
    if (dbgport) {
      eventcfg.debug = debug;
      eventcfg.dbgport = dbgport;
    } else {
      aether::log_print(aether::Runtime, "The debug port '{}' is invalid.",
                        dbgport);
      return -1;
    }
  }

  aether::MachOEngine engine{path, eventcfg};
  if (action == "encode")
    return engine.preEncode() ? 0 : -1;
  if (action == "run")
    return engine.runMain(vmargc, vmargv);

  aether::log_print(aether::Runtime, "The action '{}' doesn't support.",
                    action);
  return -1;
}

// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#include <AetherBinary.h>
#include <DarwinUtils.h>
#include <MachOEngine.h>
#include <MachOLoader.h>
#include <Utils.h>

namespace aether {

MachOEngine::MachOEngine(std::string_view path, EventConfig eventcfg)
    : MachineARM64(), BinaryEngine(this, eventcfg),
      m_path(fs::weakly_canonical(path)) {}

MachOEngine::~MachOEngine() {}

MachOType MachOEngine::machoType() {
  auto parent = m_path.parent_path();
  // .app/macho
  if (parent.extension() == ".app")
    return MachOType::iOSApp;
  // .app/Contents/MacOS/macho, otherwise as CLI program
  return parent.filename() == "MacOS" ? MachOType::MacApp : MachOType::MacCLI;
}

fs::path MachOEngine::cacheRoot() {
  auto cachedir = fs::path(getCacheDirectory()) / "MachOEngine";
  auto parent = m_path.parent_path();
  // .app/macho
  if (parent.extension() == ".app") {
    cachedir /= "iOS";
    cachedir /= parent.filename();
  }
  // .app/Contents/MacOS/macho
  else if (parent.filename() == "MacOS") {
    cachedir /= "Mac";
    cachedir /= parent.parent_path().parent_path().filename();
  } else {
    cachedir /= "CLI";
  }

  if (!fs::exists(cachedir))
    fs::create_directories(cachedir);
  return cachedir;
}

bool MachOEngine::hasWXPermission() {
#if TARGET_OS_OSX
  return machoType() != MachOType::iOSApp;
#else
  return false;
#endif
}

bool MachOEngine::preEncode() {
  std::vector<std::string> machos;
  if (machoType() == MachOType::MacCLI) {
    machos.push_back(m_path.string());
  } else {
    for (auto entry : fs::recursive_directory_iterator(m_path.parent_path())) {
      auto path = entry.path().string();
      if (entry.is_regular_file() && validMachO(path))
        machos.push_back(path);
    }
  }

  auto haswx = hasWXPermission();
  auto cachedir = cacheRoot();
  int total = 0;
  for (auto &path : machos) {
    auto bin = aether::New(path.c_str());
    if (!bin) {
      log_print(RuntimeRaw, "Failed to parse {}.", path);
      continue;
    }
    auto aevm = (cachedir / fs::path(path).filename()).string() + ".aevm";
    log_print(RuntimeRaw, "Encoding {} to {}...", path, aevm);
    BinaryEngine::preEncode(bin, aevm, haswx);
    aether::Delete(bin);
    ++total;
  }

  log_print(RuntimeRaw, "Totally encoded {} macho files.", total);
  return total > 0;
}

static const OnLoadedModule *on_loaded_module_callback;

void notify_loaded_module(std::string_view path, uint64_t base, uint64_t end) {
  auto msg = (*on_loaded_module_callback)(path, base, end);
  if (!msg.empty())
    log_print(RuntimeRaw, "When loading module {}: {}", path, msg);
}

addr_t MachOEngine::recursiveLoad(const OnLoadedModule &on_loaded_module) {
  on_loaded_module_callback = &on_loaded_module;

  auto handle = aether_dlopen(m_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    auto err = aether_dlerror();
    log_print(RuntimeRaw, "Failed to load {}: {}", m_path.string(),
              err ? err : "unknown error");
    return 0;
  }
  auto entry = aether_dlentry(handle);
  if (!entry)
    log_print(RuntimeRaw, "No entry point found in {}.", m_path.string());
  return reinterpret_cast<addr_t>(entry);
}

void MachOEngine::Unload() {
  aether_dlclose_all();
  BinaryEngine::Unload();
}

int MachOEngine::runMain(int argc, const char *argv[]) {
  auto cacheroot = cacheRoot().string();
  return BinaryEngine::runMain(argc, argv, cacheroot);
}

} // namespace aether

int engine_main(int argc, const char *argv[], int vmargc,
                const char *vmargv[]) {
  int dbgport = 61012; // defined on 12/Oct/2026
  if (argc == 1) {
    aether::log_print(aether::RuntimeRaw,
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
    aether::log_print(aether::RuntimeRaw, "The path '{}' doesn't exist.", path);
    return -1;
  }

  aether::EventConfig eventcfg;
  if (debug) {
    if (dbgport) {
      eventcfg.debug = debug;
      eventcfg.dbgport = dbgport;
    } else {
      aether::log_print(aether::RuntimeRaw, "The debug port '{}' is invalid.",
                        dbgport);
      return -1;
    }
  }

  aether::MachOEngine engine{path, eventcfg};
  if (action == "encode")
    return engine.preEncode() ? 0 : -1;
  if (action == "run")
    return engine.runMain(vmargc, vmargv);

  aether::log_print(aether::RuntimeRaw, "The action '{}' isn't supported.",
                    action);
  return -1;
}

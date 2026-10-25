// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#pragma once

#include <AetherArch.h>
#include <AetherVM.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace aether {

enum class MachOType {
  MacCLI,
  MacApp,
  iOSApp,
};

class AETHER_VMAPI MachOEngine : public MachineARM64, public BinaryEngine {
public:
  MachOEngine(std::string_view path, EventConfig eventcfg = EventConfig{});
  virtual ~MachOEngine();

  // Recursively pre-encode the main macho and its dependencies.
  bool preEncode();

  int runMain(int argc, const char *argv[]);

  MachOType machoType();
  fs::path cacheRoot();
  bool hasWXPermission();

protected:
  addr_t recursiveLoad(const OnLoadedModule &on_loaded_module) override;
  void Unload() override;

protected:
  fs::path m_path;
};

} // namespace aether

extern "C" {
AETHER_VMAPI int engine_main(int argc, const char *argv[], int vmargc,
                             const char *vmargv[]);
}

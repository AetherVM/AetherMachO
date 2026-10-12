// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.
//

#include <filesystem>
#include <thread>

#include <dlfcn.h>

extern "C" void start_macho_engine(const char *program) {
  auto macho_engine =
      std::filesystem::path(program).parent_path() / "lib/libMachOEngine.dylib";
  auto path = macho_engine.string();

  printf("Loading %s...\n", path.data());
  auto handle = dlopen(path.data(), RTLD_NOW);
  if (handle) {
    // The server starts running automatically by its module ctors
    printf("The MachO Engine is running...\n");
  } else {
    printf("Failed to load %s: %s.\n", path.data(), dlerror());
  }
}

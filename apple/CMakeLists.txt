# AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
# Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
# SPDX-License-Identifier: Apache License, Version 2.0
# See LICENSE file in the root directory for full license text.

add_library(MachOLoader STATIC
  dyld/dyldMain.cpp
)

target_compile_definitions(MachOLoader PRIVATE
  TARGET_OS_EXCLAVEKIT=0
  BUILDING_DYLD=1
)

target_include_directories(MachOLoader PRIVATE
  cache_builder
  common
  dyld
  include
  include/mach-o
  libdyld
  lsl
  mach_o
  mach_o_writer
)

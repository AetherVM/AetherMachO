// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#pragma once

#include <AetherArch.h>
#include <AetherVM.h>

namespace aether {

class MachOEngine : public BinaryEngine {
public:
  MachOEngine(EventConfig eventcfg = EventConfig{});
  ~MachOEngine();

protected:
  MachineARM64 m_machine;
};

} // namespace aether

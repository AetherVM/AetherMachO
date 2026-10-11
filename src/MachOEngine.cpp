// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#include <MachOEngine.h>

namespace aether {

MachOEngine::MachOEngine(EventConfig eventcfg)
    : BinaryEngine(&m_machine, eventcfg) {}

MachOEngine::~MachOEngine() {}

} // namespace aether

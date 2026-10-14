// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#pragma once

#include <Common.h>
#include <TargetConditionals.h>

#include <string>

namespace aether {

AETHER_VMAPI std::string getCacheDirectory();
AETHER_VMAPI bool validMachO(std::string_view path);

} // namespace aether

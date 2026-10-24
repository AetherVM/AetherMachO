// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: APSL Version 2.0
// See APPLE_LICENSE file in the root directory for full license text.

#pragma once

#include <dlfcn.h>

void *aether_dlopen(const char *path, int mode);
void *aether_dlsym(void *handle, const char *symbol);
char *aether_dlerror();
int aether_dladdr(const void *addr, Dl_info *info);
int aether_dlclose(void *handle);
bool aether_dlopen_preflight(const char *path);

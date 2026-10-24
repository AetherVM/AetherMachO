// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: APSL Version 2.0
// See APPLE_LICENSE file in the root directory for full license text.

#include "MachOLoader.h"

#include <DyldAPIs.h>
#include <DyldDelegates.h>
#include <DyldProcessConfig.h>
#include <crt_externs.h>
#include <mach-o/ldsyms.h>
#include <new>

using dyld4::APIs;
using dyld4::KernelArgs;
using dyld4::ProcessConfig;
using dyld4::RuntimeLocks;
using dyld4::SyscallDelegate;
using lsl::Allocator;

namespace {

// Global APIs instance, created once by the constructor below.
APIs *gAPIs = nullptr;

KernelArgs gKernelArgs;
SyscallDelegate gSyscallDelegate;

// Module initializers receive the process arguments from dyld, and
// ProcessConfig expects them packed as argv[], envp[], apple[] in a KernelArgs
// block.
__attribute__((constructor)) void initializeAPIs(int argc, const char *argv[],
                                                 const char *envp[],
                                                 const char *apple[]) {
  if (gAPIs)
    return;
  size_t n = 0;
  auto append = [&](const char *s) {
    if (n < MAX_KERNEL_ARGS)
      gKernelArgs.args[n++] = s;
  };
  for (int i = 0; i < argc; ++i)
    append(argv[i]);
  append(nullptr);
  for (const char **e = envp; e && *e; ++e)
    append(*e);
  append(nullptr);
  for (const char **a = apple; a && *a; ++a)
    append(*a);
  append(nullptr);
  gKernelArgs.argc = static_cast<uintptr_t>(argc);
  gKernelArgs.mainExecutable =
      reinterpret_cast<const dyld3::MachOAnalyzer *>(_NSGetMachExecuteHeader());

  Allocator &allocator = lsl::MemoryManager::defaultAllocator();
  // RuntimeLocks must live outside the allocator pool, which may be read-only.
  static uint8_t locksStorage[sizeof(RuntimeLocks)]
      __attribute__((aligned(alignof(RuntimeLocks))));
  RuntimeLocks &locks = *new (locksStorage) RuntimeLocks();

  lsl::MemoryManager::withWritableMemory([&] {
    ProcessConfig &config =
        *new (allocator.aligned_alloc(alignof(ProcessConfig),
                                      sizeof(ProcessConfig)))
            ProcessConfig(&gKernelArgs, gSyscallDelegate, allocator);
    gAPIs = new (allocator.aligned_alloc(alignof(APIs), sizeof(APIs)))
        APIs(config, locks, allocator);
  });
}

} // namespace

void *aether_dlopen(const char *path, int mode) {
  return gAPIs ? gAPIs->dlopen(path, mode) : nullptr;
}

void *aether_dlsym(void *handle, const char *symbol) {
  return gAPIs ? gAPIs->dlsym(handle, symbol) : nullptr;
}

char *aether_dlerror() { return gAPIs ? gAPIs->dlerror() : nullptr; }

int aether_dladdr(const void *addr, Dl_info *info) {
  return gAPIs ? gAPIs->dladdr(addr, info) : 0;
}

int aether_dlclose(void *handle) { return gAPIs ? gAPIs->dlclose(handle) : -1; }

bool aether_dlopen_preflight(const char *path) {
  return gAPIs && gAPIs->dlopen_preflight(path);
}

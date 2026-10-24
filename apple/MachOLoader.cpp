// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: APSL Version 2.0
// See APPLE_LICENSE file in the root directory for full license text.

#include "MachOLoader.h"

#include <ChainedFixups.h>
#include <DyldAPIs.h>
#include <DyldDelegates.h>
#include <DyldProcessConfig.h>
#include <Loader.h>
#include <SharedCacheRuntime.h>
#include <UnsafeHeader.h>
#include <crt_externs.h>
#include <mach-o/ldsyms.h>
#include <new>
#include <ptrauth.h>

using dyld4::APIs;
using dyld4::KernelArgs;
using dyld4::ProcessConfig;
using dyld4::RuntimeLocks;
using dyld4::SyscallDelegate;
using lsl::Allocator;

extern const dyld3::MachOLoaded __dso_handle;

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

// Emulated processes never map the system dyld shared cache.
bool dyld3::loadDyldCache(const SharedCacheOptions &,
                          SharedCacheLoadInfo *results) {
  results->loadAddress = nullptr;
  results->slide = 0;
  results->errorMessage = "dyld shared cache is not supported";
  results->cacheFileFound = false;
  return false;
}

// Only the writer side encodes chains, which the loader never does.
mach_o::Error mach_o::ChainedFixups::PointerFormat::writeChainEntry(
    const Fixup &, const void *, uint64_t,
    std::span<const MappedSegment *>) const {
  return mach_o::Error("writing chained fixups is not supported");
}

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

void *aether_dlentry(void *handle) {
  if (!gAPIs || !handle)
    return nullptr;
  // Mirrors the handle encoding of APIs::dlopen().
  void *h = ptrauth_strip(handle, ptrauth_key_process_dependent_data);
  auto ldr = reinterpret_cast<const dyld4::Loader *>(
      (reinterpret_cast<uintptr_t>(h) & ~uintptr_t(1)) ^
      reinterpret_cast<uintptr_t>(&__dso_handle));
  auto base = reinterpret_cast<uintptr_t>(ldr->loadAddress(*gAPIs));
  auto hdr = reinterpret_cast<const mach_o::UnsafeHeader *>(base);
  uint64_t offset = 0;
  bool usesCRT = false;
  if (!base || !hdr->getEntry(offset, usesCRT))
    return nullptr;
  return reinterpret_cast<void *>(base + offset);
}

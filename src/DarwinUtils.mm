// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

#include <DarwinUtils.h>

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/machine.h>

#include <fstream>

#import <Foundation/Foundation.h>

namespace aether {

std::string getCacheDirectory() {
  @autoreleasepool {
    NSArray<NSString *> *paths = NSSearchPathForDirectoriesInDomains(
        NSCachesDirectory, NSUserDomainMask, YES);
    if (paths.count > 0) {
      NSString *cachesPath = paths.firstObject;
      return [cachesPath UTF8String];
    }
    return "";
  }
}

static inline uint32_t swap32(uint32_t val) { return __builtin_bswap32(val); }

bool validMachO(std::string_view path) {
  std::ifstream file(std::string(path), std::ios::binary);
  if (!file.is_open())
    return false;

  uint32_t magic = 0;
  file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
  if (!file)
    return false;

  if (magic == MH_MAGIC_64) {
    mach_header_64 header;
    file.seekg(0);
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!file)
      return false;

    return header.cputype == CPU_TYPE_ARM64;
  }

  if (magic == FAT_CIGAM_64 || magic == FAT_MAGIC_64) {
    fat_header fatHeader;
    file.seekg(0);
    file.read(reinterpret_cast<char *>(&fatHeader), sizeof(fatHeader));
    if (!file)
      return false;

    uint32_t nfat_arch = (magic == FAT_CIGAM_64) ? swap32(fatHeader.nfat_arch)
                                                 : fatHeader.nfat_arch;

    for (uint32_t i = 0; i < nfat_arch; ++i) {
      fat_arch_64 arch64;
      file.read(reinterpret_cast<char *>(&arch64), sizeof(arch64));
      if (!file)
        break;

      cpu_type_t cputype =
          (magic == FAT_CIGAM_64) ? swap32(arch64.cputype) : arch64.cputype;
      if (cputype == CPU_TYPE_ARM64)
        return true;
    }
  }

  return false;
}

} // namespace aether

#pragma once

#include <stdint.h>

namespace dyld3 {

// Maps a version set to the equivalent OS version of each platform.
// Entries must be sorted by ascending set.
struct VersionSetEntry {
  uint32_t set;
  uint32_t macos;
  uint32_t ios;
  uint32_t watchos;
  uint32_t tvos;
  uint32_t bridgeos;
  uint32_t visionos;
};

static const VersionSetEntry sVersionMap[] = {
    {0x07050000, 0x000A0E00, 0x000C0000, 0x00050000, 0x000C0000, 0x00030000, 0},
    {0x07060000, 0x000A0F00, 0x000D0000, 0x00060000, 0x000D0000, 0x00040000, 0},
    {0x07070000, 0x000B0000, 0x000E0000, 0x00070000, 0x000E0000, 0x00050000, 0},
    {0x07080000, 0x000C0000, 0x000F0000, 0x00080000, 0x000F0000, 0x00060000, 0},
    {0x07090000, 0x000D0000, 0x00100000, 0x00090000, 0x00100000, 0x00070000, 0},
    {0x070A0000, 0x000E0000, 0x00110000, 0x000A0000, 0x00110000, 0x00080000, 0x00010000},
    {0x070B0000, 0x000F0000, 0x00120000, 0x000B0000, 0x00120000, 0x00090000, 0x00020000},
};

} // namespace dyld3

#pragma once

#include <mach-o/loader.h>
#include <stdint.h>

// Older name of PLATFORM_MACCATALYST, still used by dyld sources.
#ifndef PLATFORM_IOSMAC
#define PLATFORM_IOSMAC PLATFORM_MACCATALYST
#endif

enum {
  AMFI_DYLD_INPUT_PROC_IN_SIMULATOR = (1 << 0),
  AMFI_DYLD_INPUT_PROC_HAS_RESTRICT_SEG = (1 << 1),
  AMFI_DYLD_INPUT_PROC_IS_ENCRYPTED = (1 << 2),
};

enum amfi_dyld_policy_output_flag_set {
  AMFI_DYLD_OUTPUT_ALLOW_AT_PATH = (1 << 0),
  AMFI_DYLD_OUTPUT_ALLOW_PATH_VARS = (1 << 1),
  AMFI_DYLD_OUTPUT_ALLOW_CUSTOM_SHARED_CACHE = (1 << 2),
  AMFI_DYLD_OUTPUT_ALLOW_FALLBACK_PATHS = (1 << 3),
  AMFI_DYLD_OUTPUT_ALLOW_PRINT_VARS = (1 << 4),
  AMFI_DYLD_OUTPUT_ALLOW_FAILED_LIBRARY_INSERTION = (1 << 5),
  AMFI_DYLD_OUTPUT_ALLOW_LIBRARY_INTERPOSING = (1 << 6),
  AMFI_DYLD_OUTPUT_ALLOW_EMBEDDED_VARS = (1 << 7),
  AMFI_DYLD_OUTPUT_ALLOW_DEVELOPMENT_VARS = (1 << 8),
  AMFI_DYLD_OUTPUT_ALLOW_LIBSYSTEM_OVERRIDE = (1 << 9),
};

// The engine runs inside a regular host process, so no restriction applies.
static inline int amfi_check_dyld_policy_self(uint64_t query_flags,
                                              uint64_t *result_flags) {
  (void)query_flags;
  *result_flags = (1 << 10) - 1;
  return 0;
}

#pragma once

enum sandbox_filter_type {
  SANDBOX_FILTER_PATH = 1,
  SANDBOX_FILTER_SYSCALL_NUMBER = 8,
  SANDBOX_CHECK_NO_REPORT = 0x40000000,
};

// No sandbox policy applies to the host process.
static inline int sandbox_check(int, const char *, int, ...) { return 0; }

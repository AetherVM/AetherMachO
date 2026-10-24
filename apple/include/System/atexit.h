#pragma once

#include <stdint.h>

struct __cxa_range_t {
  const void *addr;
  uintptr_t length;
};

void __cxa_finalize_ranges(const struct __cxa_range_t ranges[],
                           unsigned int count);

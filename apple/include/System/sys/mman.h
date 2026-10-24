#pragma once
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>

static inline int mremap_encrypted(void *, size_t, uint32_t, uint32_t,
                                   uint32_t) {
  errno = ENOTSUP;
  return -1;
}

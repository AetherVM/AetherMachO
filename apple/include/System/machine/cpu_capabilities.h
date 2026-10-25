#pragma once

// Included by assembly sources too, which only need the header to exist.
#ifndef __ASSEMBLER__

#include <stdint.h>

// The kernel comm page is not mapped into the host process, so every comm page
// field is backed by a zeroed scratch area instead.
static inline uintptr_t aether_comm_page_scratch() {
  static uint64_t scratch[8];
  return (uintptr_t)scratch;
}

#define _COMM_PAGE_DYLD_FLAGS aether_comm_page_scratch()
#define _COMM_PAGE_DTRACE_DOF_ENABLED aether_comm_page_scratch()
#define _COMM_PAGE_DEV_FIRM aether_comm_page_scratch()

#endif // __ASSEMBLER__

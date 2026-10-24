#pragma once

#define CSR_ALLOW_APPLE_INTERNAL (1 << 4)

// SIP state is not queried; the host process is never treated as internal.
static inline int csr_check(unsigned int) { return -1; }

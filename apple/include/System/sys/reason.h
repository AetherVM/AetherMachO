#pragma once

#include <fcntl.h>

#define OS_REASON_DYLD 6

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// MachOEngine added
static inline __attribute__((noreturn)) void abort_with_payload(
    uint32_t reason_namespace, uint64_t reason_code, void *payload,
    uint32_t payload_size, const char *reason_string, uint64_t reason_flags) {
    (void)reason_namespace; (void)reason_code; (void)payload;
    (void)payload_size; (void)reason_string; (void)reason_flags;
		puts(reason_string);
    abort();
}

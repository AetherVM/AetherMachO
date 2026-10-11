#pragma once

#define KDBG_CODE(...) 0
#define DBG_DYLD_UUID_MAP_A 0
#define DBG_DYLD_UUID_UNMAP_A 0

static inline bool kdebug_is_enabled(uint32_t code) {
    (void)code;
    return false;
}

static inline int kdebug_trace(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    (void)code; (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    return 0;
}

static inline uint64_t kdebug_trace_string(uint32_t code, uint64_t trace_id, const char *str) {
    (void)code; (void)trace_id; (void)str;
    return 0;
}
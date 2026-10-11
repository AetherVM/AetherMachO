#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int amfi_check_dyld_policy_self(uint64_t query_flags, uint64_t *result_flags);

#ifdef __cplusplus
}
#endif

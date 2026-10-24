#pragma once

// Switching the process-wide image info is not supported by the host kernel.
static inline int proc_set_dyld_all_image_info(void *, unsigned long) { return -1; }

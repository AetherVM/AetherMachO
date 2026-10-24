#pragma once

#include <stdint.h>
#include <sys/ioctl.h>

#define DTRACE_MODNAMELEN 64
#define DTRACEMNR_HELPER "dtracehelper"
#define DTRACEHIOC (((unsigned)'h') << 8)
#define DTRACEHIOC_ADD (DTRACEHIOC | 1)
#define DTRACEHIOC_REMOVE (DTRACEHIOC | 2)
#define DTRACEHIOC_ADDDOF (DTRACEHIOC | 4)

typedef struct dof_helper {
  char dofhp_mod[DTRACE_MODNAMELEN];
  uint64_t dofhp_addr;
  uint64_t dofhp_dof;
} dof_helper_t;

typedef struct dof_ioctl_data {
  uint64_t dofiod_count;
  dof_helper_t dofiod_helpers[1];
} dof_ioctl_data_t;

#pragma once

#include <sys/attr.h>
#include <sys/fsgetpath.h>
#include <sys/mount.h>

#ifdef __cplusplus
extern "C" {
#endif
int openbyid_np(fsid_t *fsid, fsobj_id_t *objid, int flags);
#ifdef __cplusplus
}
#endif

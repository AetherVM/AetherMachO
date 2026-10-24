#pragma once

#include <fcntl.h>
#include <sys/stat.h>

// Subsystem-aware file APIs are not needed outside the OS; use the plain ones.
static inline int stat_with_subsystem(const char *path, struct stat *buf) {
  return ::stat(path, buf);
}

static inline int open_with_subsystem(const char *path, int flags) {
  return ::open(path, flags);
}

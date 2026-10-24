#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _simple_getenv(vendor, env) getenv(env)

// Minimal malloc-backed replacement for libplatform's _simple string API.
struct _SIMPLE {
  char *buf;
  size_t len;
};
typedef void *_SIMPLE_STRING;

static inline _SIMPLE_STRING _simple_salloc(void) {
  _SIMPLE *s = (_SIMPLE *)malloc(sizeof(_SIMPLE));
  if (!s)
    return nullptr;
  s->buf = (char *)calloc(1, 1);
  s->len = 0;
  if (!s->buf) {
    free(s);
    return nullptr;
  }
  return s;
}

static inline int _simple_vsprintf(_SIMPLE_STRING handle, const char *fmt,
                                   va_list ap) {
  _SIMPLE *s = (_SIMPLE *)handle;
  va_list copy;
  va_copy(copy, ap);
  int n = vsnprintf(nullptr, 0, fmt, copy);
  va_end(copy);
  if (n < 0)
    return -1;
  char *grown = (char *)realloc(s->buf, s->len + (size_t)n + 1);
  if (!grown)
    return -1;
  s->buf = grown;
  vsnprintf(s->buf + s->len, (size_t)n + 1, fmt, ap);
  s->len += (size_t)n;
  return n;
}

static inline char *_simple_string(_SIMPLE_STRING handle) {
  return ((_SIMPLE *)handle)->buf;
}

static inline char *_simple_sresize(_SIMPLE_STRING handle) {
  _SIMPLE *s = (_SIMPLE *)handle;
  s->len = strlen(s->buf);
  return s->buf;
}

static inline void _simple_sfree(_SIMPLE_STRING handle) {
  _SIMPLE *s = (_SIMPLE *)handle;
  if (!s)
    return;
  free(s->buf);
  free(s);
}

static inline int _simple_vsnprintf(char *buf, size_t size, const char *fmt,
                                    va_list ap) {
  return vsnprintf(buf, size, fmt, ap);
}

static inline int _simple_vdprintf(int fd, const char *fmt, va_list ap) {
  return vdprintf(fd, fmt, ap);
}

static inline int _simple_dprintf(int fd, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int n = vdprintf(fd, fmt, ap);
  va_end(ap);
  return n;
}

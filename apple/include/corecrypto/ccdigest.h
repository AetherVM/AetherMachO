#pragma once

// Minimal corecrypto shim backed by CommonCrypto.
#include <CommonCrypto/CommonDigest.h>
#include <stddef.h>
#include <stdint.h>

#define CCSHA1_OUTPUT_SIZE 20
#define CCSHA256_OUTPUT_SIZE 32
#define CCSHA384_OUTPUT_SIZE 48

struct ccdigest_info {
  int kind; // output size in bytes
};

union cc_shim_ctx {
  CC_SHA1_CTX sha1;
  CC_SHA256_CTX sha256;
  CC_SHA512_CTX sha512;
};

static inline const struct ccdigest_info *ccsha1_di(void) {
  static const struct ccdigest_info di = {CCSHA1_OUTPUT_SIZE};
  return &di;
}
static inline const struct ccdigest_info *ccsha256_di(void) {
  static const struct ccdigest_info di = {CCSHA256_OUTPUT_SIZE};
  return &di;
}
static inline const struct ccdigest_info *ccsha384_di(void) {
  static const struct ccdigest_info di = {CCSHA384_OUTPUT_SIZE};
  return &di;
}

#define ccdigest_di_decl(di, name) union cc_shim_ctx name
#define ccdigest_di_clear(di, name) memset(&(name), 0, sizeof(name))

#define ccdigest_init(di, ctx)                                                 \
  do {                                                                         \
    switch ((di)->kind) {                                                      \
    case CCSHA1_OUTPUT_SIZE:                                                   \
      CC_SHA1_Init(&(ctx).sha1);                                               \
      break;                                                                   \
    case CCSHA256_OUTPUT_SIZE:                                                 \
      CC_SHA256_Init(&(ctx).sha256);                                           \
      break;                                                                   \
    default:                                                                   \
      CC_SHA384_Init(&(ctx).sha512);                                           \
    }                                                                          \
  } while (0)

#define ccdigest_update(di, ctx, len, data)                                    \
  do {                                                                         \
    switch ((di)->kind) {                                                      \
    case CCSHA1_OUTPUT_SIZE:                                                   \
      CC_SHA1_Update(&(ctx).sha1, (data), (CC_LONG)(len));                     \
      break;                                                                   \
    case CCSHA256_OUTPUT_SIZE:                                                 \
      CC_SHA256_Update(&(ctx).sha256, (data), (CC_LONG)(len));                 \
      break;                                                                   \
    default:                                                                   \
      CC_SHA384_Update(&(ctx).sha512, (data), (CC_LONG)(len));                 \
    }                                                                          \
  } while (0)

#define ccdigest_final(di, ctx, out)                                           \
  do {                                                                         \
    switch ((di)->kind) {                                                      \
    case CCSHA1_OUTPUT_SIZE:                                                   \
      CC_SHA1_Final((out), &(ctx).sha1);                                       \
      break;                                                                   \
    case CCSHA256_OUTPUT_SIZE:                                                 \
      CC_SHA256_Final((out), &(ctx).sha256);                                   \
      break;                                                                   \
    default:                                                                   \
      CC_SHA384_Final((out), &(ctx).sha512);                                   \
    }                                                                          \
  } while (0)

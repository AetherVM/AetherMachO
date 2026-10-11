#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Private Apple libpthread function for initializing pre-reserved TLS keys
int pthread_key_init_np(int key, void (*destructor)(void *));

#ifdef __cplusplus
}
#endif

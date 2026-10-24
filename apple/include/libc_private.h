#pragma once

#ifdef __cplusplus
extern "C" {
#endif
void abort_report_np(const char *format, ...)
    __attribute__((noreturn, format(printf, 1, 2)));
#ifdef __cplusplus
}
#endif

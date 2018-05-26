#ifndef JOS_STDIO_H_
#define JOS_STDIO_H_

#include <stdarg.h>

#ifdef JOS
#ifdef KERNEL
#include "kernel/printk.h"
#define printf printk
#else  // KERNEL
#define printf printu
#endif  // KERNEL
#endif  // JOS

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

int printu(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_STDIO_H_

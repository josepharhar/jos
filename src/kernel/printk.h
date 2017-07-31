#ifndef PRINTK_H_
#define PRINTK_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int printk(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
int vprintk(const char* format, va_list list);

#ifdef __cplusplus
}
#endif

#endif  // PRINTK_H_

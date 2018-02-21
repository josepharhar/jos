#ifndef JOS_STDIO_H_
#define JOS_STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdarg.h>

int printj(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_STDIO_H_

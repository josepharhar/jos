#ifndef VPRINTF_H_
#define VPRINTF_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CharPrinter)(char character);
typedef void (*StringPrinter)(const char* string);

int vprintf(const char* format,
            va_list list,
            CharPrinter char_printer,
            StringPrinter string_printer);

#ifdef __cplusplus
}
#endif

#endif  // VPRINTF_H_

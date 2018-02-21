#include "stdio.h"

#include <stdarg.h>

#include "getc.h"
#include "vprintf.h"

int printj(const char* format, ...) {
  va_list list;
  va_start(list, format);
  int ret_value = vprintf(format, list, Putc, Puts);
  va_end(list);
  return ret_value;
}

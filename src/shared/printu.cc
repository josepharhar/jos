#include "printu.h"

#include <stdarg.h>

#include "io.h"
#include "vprintf.h"

int printu(const char* format, ...) {
  va_list list;
  va_start(list, format);
  int ret_value = vprintf(format, list, Putc, Puts);
  va_end(list);
  return ret_value;
}

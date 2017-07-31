#include "dcheck.h"

#include "printu.h"

void DCHECKFailed(const char* condition) {
  printu("DCHECK failed: %s\n", condition);
}

void DCHECKFailed(const char* condition, const char* format, ...) {
  printu("DCHECK failed: %s\n  ", condition);
  va_list list;
  va_start(list, format);
  vprintu(format, list);
  va_end(list);
  printu("\n");
}

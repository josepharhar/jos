#include "stdcc.h"

#define UNIMPLEMENTED()   \
  while (1) {             \
    asm volatile ("hlt"); \
  }

void __cxa_pure_virtual() {
  UNIMPLEMENTED();
}

void __dso_handle() {
  UNIMPLEMENTED();
}

void __cxa_atexit() {
  UNIMPLEMENTED();
}

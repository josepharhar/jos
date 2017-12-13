#include "knew.h"

#include "kernel/kmalloc.h"

void* operator new(uint64_t size) {
  return kcalloc(size);
}

void operator delete(void* address) {
  kfree(address);
}

void operator delete(void* address, uint64_t size) {
  kfree(address);
}

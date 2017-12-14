// This file provides symbols needed for linking C++, which may be different for
// the kernel executable, user executables, and test executables.

#include "shared/stdint.h"

#ifdef KERNEL

#include "kernel/kmalloc.h"
#include "kernel/printk.h"

#define NOTIMPLEMENTED()                            \
  printk("NOTIMPLEMENTED %s", __PRETTY_FUNCTION__); \
  while (1) {                                       \
    asm volatile("hlt");                            \
  }

#else  // user executable

#include "shared/printu.h"

#define NOTIMPLEMENTED() printu("NOTIMPLEMENTED %s", __PRETTY_FUNCTION__);

#endif  // KERNEL

static void* allocate(uint64_t size) {
#ifdef KERNEL
  return kmalloc(size);
#else
  NOTIMPLEMENTED();
  return 0;
#endif
}
static void* callocate(uint64_t size) {
#ifdef KERNEL
  return kcalloc(size);
#else
  NOTIMPLEMENTED();
  return 0;
#endif
}
static void deallocate(void* address) {
#ifdef KERNEL
  kfree(address);
#else
  NOTIMPLEMENTED();
#endif
}

void* operator new(uint64_t size) {
  return allocate(size);
}
void* operator new[](uint64_t size) {
  return allocate(size);
}
void operator delete(void* address) {
  deallocate(address);
}
void operator delete(void* address, uint64_t size) {
  deallocate(address);
}
void operator delete[](void* address) {
  deallocate(address);
}

extern "C" {
void* malloc(uint64_t size) {
  return allocate(size);
}
void* calloc(uint64_t size) {
  return callocate(size);
}
void free(void* address) {
  deallocate(address);
}

// Called when a pure virtual function is called without being filled in.
void __cxa_pure_virtual() {
  NOTIMPLEMENTED();
}

// Dynamic Shared Object Handle. ...what?
void* __dso_handle = 0;

// Called to register a destructor that should be called when a shared library
// needs to be unloaded.
void __cxa_atexit() {
  NOTIMPLEMENTED();
}

}  // extern "C"

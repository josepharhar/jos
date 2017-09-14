#ifndef KPROCMALLOC_H_
#define KPROCMALLOC_H_

#include "stdint.h"

// Allocator for process-specific kernel memory
void* kprocmalloc(uint64_t size);
void kprocfree(void* address);
void* kproccalloc(uint64_t size);

#endif  // KPROCMALLOC_H_

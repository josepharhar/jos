#ifndef KMALLOC_H_
#define KMALLOC_H_

#include "stdint.h"

void* kmalloc(uint64_t size);
void kfree(void* address);

void* kcalloc(uint64_t size);

#endif  // KMALLOC_H_

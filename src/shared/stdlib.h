#ifndef SHARED_STDLIB_H_
#define SHARED_STDLIB_H_

#include "stdint.h"

extern "C" {

void* malloc(uint64_t num_bytes);
void* calloc(uint64_t num_bytes);
void free(void* address);

}  // extern "C"

#endif  // SHARED_STDLIB_H_

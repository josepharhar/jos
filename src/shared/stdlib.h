#ifndef SHARED_STDLIB_H_
#define SHARED_STDLIB_H_

void* malloc(uint64_t num_bytes);
void* calloc(uint64_t num_bytes);
void free(void* address);

#endif  // SHARED_STDLIB_H_

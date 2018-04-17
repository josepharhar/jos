#include "umalloc.h"

#include "string.h"

void* umalloc(int size) {
  // TODO write sbrk() and make a real user memory allocator
  static uint64_t next_malloc = 0x0000090000000000;
  void* return_value = (void*)next_malloc;
  next_malloc += size;
  next_malloc += (next_malloc % 16);
  return return_value;
}

void ufree(void* ptr){
  // TODO
}

void* ucalloc(int size){
  void* ptr = umalloc(size);
  memset(ptr, 0, size);
  return ptr;
}

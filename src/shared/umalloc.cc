#include "umalloc.h"

#include "string.h"

void* umalloc(int size) {
  // TODO
  static uint64_t next_malloc = 0x9000000000000000;
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

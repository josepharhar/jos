#ifndef SHARED_CLONE_H_
#define SHARED_CLONE_H_

#include "stdint.h"

#define CLONE_FILES (1 << 0) // copy fd table
#define CLONE_VM (1 << 1) // don't copy page table

struct CloneOptions {
  bool copy_page_table;
  bool copy_fds;
  uint64_t callback;
  uint64_t new_stack;
} __attribute__((packed));

typedef void (*CloneCallback)();

void clone(CloneCallback callback, void* new_stack, int flags);

#endif  // SHARED_CLONE_H_

#ifndef SHARED_CLONE_H_
#define SHARED_CLONE_H_

#include "stdint.h"
#include "unistd.h"

#define CLONE_FILES (1 << 0) // copy fd table
#define CLONE_VM (1 << 1) // don't copy page table

struct SyscallCloneParams {
  bool copy_page_table;
  bool copy_fds;
  uint64_t callback;
  uint64_t new_stack;

  uint64_t pid_writeback;
};

typedef void (*CloneCallback)();

pid_t clone(CloneCallback callback, void* new_stack, int flags);

#endif  // SHARED_CLONE_H_

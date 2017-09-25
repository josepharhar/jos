#include "clone.h"

#include "syscall.h"

void clone(CloneOptions options, CloneCallback callback, void* new_stack) {
  Syscall(SYSCALL_CLONE, options.Serialize(), (uint64_t)callback,
          (uint64_t)new_stack);
}

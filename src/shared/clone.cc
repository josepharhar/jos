#include "clone.h"

#include "syscall.h"

void clone(CloneCallback callback, void* new_stack) {
  Syscall(SYSCALL_CLONE, (uint64_t) callback, (uint64_t) new_stack);
}

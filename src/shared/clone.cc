#include "clone.h"

#include "syscall.h"

void clone(CloneCallback callback) {
  Syscall(SYSCALL_CLONE, (uint64_t) callback);
}

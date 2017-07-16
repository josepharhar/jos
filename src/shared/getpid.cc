#include "getpid.h"

#include "syscall.h"

uint64_t getpid() {
  static uint64_t pid;
  Syscall(SYSCALL_GETPID, (uint64_t) &pid);
  return pid;
}

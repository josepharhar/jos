#include "unistd.h"

#include "clone.h"
#include "syscall.h"

pid_t fork() {
  return clone(0, 0, CLONE_FILES);
}

pid_t getpid() {
  static uint64_t pid;
  Syscall(SYSCALL_GETPID, (uint64_t)&pid);
  return (int)pid;
}

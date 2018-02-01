#include "shared/exit.h"

#include "shared/syscall.h"
#include "printu.h"

// TODO exit_void shouldn't exit and printu should not be used in a shared lib

void exit_void() {
  exit(0);
}

void exit(int exit_code) {
  // TODO how does casting signed exit_code to uint64_t work?
  printu("exit(%d)\n", exit_code);
  Syscall(SYSCALL_EXIT, (uint64_t)exit_code, 0, 0);
}

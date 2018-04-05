#include "exec.h"

#include "syscall.h"

void exec(const char* filename) {
  Syscall(SYSCALL_EXEC, (uint64_t)filename);
}

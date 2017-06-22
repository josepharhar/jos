#include "exec.h"

#include "syscall.h"

void Exec(char* filename) {
  Syscall(SYSCALL_EXEC, (uint64_t) filename);
}

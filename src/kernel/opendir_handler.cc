#include "opendir_handler.h"

#include "syscall.h"
#include "syscall_handler.h"
#include "shared/dirent.h"

static void HandleOpendir(uint64_t interrupt_number,
                          uint64_t param_1,
                          uint64_t param_2,
                          uint64_t param_3) {
  SyscallOpendirParams* params = (SyscallOpendirParams*)param_1;
}

void InitOpendir() {
  SetSyscallHandler(SYSCALL_OPENDIR, HandleOpendir);
}

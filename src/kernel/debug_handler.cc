#include "debug_handler.h"

#include "proc.h"
#include "syscall.h"
#include "syscall_handler.h"

static void HandleSyscallDebug(uint64_t interrupt_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3) {
  proc::Print();
}

void InitDebug() {
  SetSyscallHandler(SYSCALL_DEBUG, HandleSyscallDebug);
}

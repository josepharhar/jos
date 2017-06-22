#include "fork_handler.h"

#include "syscall_handler.h"
#include "proc.h"
#include "syscall.h"

static void HandleSyscallFork(uint64_t interrupt_number, uint64_t param_1, uint64_t param_2, uint64_t param_3) {
  ProcContext* child_proc = ProcCreateCopy(param_2);

  // param_1 is address of pid
  // TODO access this memory safely with security checks
  uint64_t* pid = (uint64_t*) param_1;
  *pid = child_proc->pid;
}

void InitFork() {
  SetSyscallHandler(SYSCALL_FORK, HandleSyscallFork);
}

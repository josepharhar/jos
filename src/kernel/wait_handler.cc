#include "wait_handler.h"

#include "syscall.h"
#include "syscall_handler.h"
#include "proc.h"

static void HandleSyscallWait(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  SyscallWaitParams* params = (SyscallWaitParams*)param_1;

  proc::ProcContext* current_proc = proc::GetCurrentProc();
  current_proc->wait_for_zombie = new proc::BlockedQueue();
  current_proc->wait_for_zombie->BlockCurrentProcNoNesting();
  current_proc->wait_params = params;

  proc::TryFinishWaiting(current_proc);
}

void InitWait() {
  SetSyscallHandler(SYSCALL_WAIT, HandleSyscallWait);
}

#include "getpid_handler.h"

#include "syscall_handler.h"
#include "syscall.h"
#include "proc.h"
#include "printk.h"

static void HandleSyscallGetpid(uint64_t interrupt_number,
                                uint64_t param_1,
                                uint64_t param_2,
                                uint64_t param_3) {
  // TODO security check userspace pointer
  uint64_t* pid_pointer = (uint64_t*)param_1;
  *pid_pointer = proc::GetCurrentPid();
  printk("HandleSyscallGetpid() %p: %d\n", pid_pointer, proc::GetCurrentPid());
}

void InitGetpid() {
  SetSyscallHandler(SYSCALL_GETPID, HandleSyscallGetpid);
}

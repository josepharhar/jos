#include "syscall_handler.h"

#include "proc.h"
#include "printk.h"
#include "syscall.h"

static void DefaultSyscallHandler(uint64_t syscall_number,
                                  uint64_t param_1,
                                  uint64_t param_2,
                                  uint64_t param_3) {
  printk("DefaultSyscallHandler() syscall_number: %lld\n", syscall_number);
}

static SyscallHandler syscall_handlers[MAX_NUM_SYSCALLS];

void InitSyscall() {
  for (int i = 0; i < MAX_NUM_SYSCALLS; i++) {
    syscall_handlers[i] = DefaultSyscallHandler;
  }
}

void SetSyscallHandler(uint64_t syscall_number, SyscallHandler handler) {
  if (syscall_number < MAX_NUM_SYSCALLS) {
    syscall_handlers[syscall_number] = handler;
  } else {
    printk("syscall number too large to use %lld\n", syscall_number);
  }
}

// this is called from interrupt handler
// interrupt_number and error_code are just there to fill in rdi and rsi
// this is called with interrupts disabled, it is a trap not an interrupt
void HandleSyscall(uint64_t interrupt_number,
                   uint64_t error_code,
                   uint64_t syscall_number,
                   uint64_t param_1,
                   uint64_t param_2,
                   uint64_t param_3) {
  Proc::SaveStateToCurrentProc();

  if (syscall_number < MAX_NUM_SYSCALLS) {
    syscall_handlers[syscall_number](syscall_number, param_1, param_2, param_3);
  } else {
    printk("syscall number too large to handle: %lld\n", syscall_number);
  }
}

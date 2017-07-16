#include "clone_handler.h"

#include "syscall.h"
#include "clone.h"
#include "proc.h"
#include "syscall_handler.h"

static void HandleSyscallClone(uint64_t interrupt_number,
                               uint64_t callback,
                               uint64_t param_2,
                               uint64_t param_3) {
  ProcContext* new_proc = ProcClone(callback);

  // TODO set new page table for new proc
  //new_proc->cr3 = (uint64_t) CopyCurrentPageTable();
}

void InitClone() {
  SetSyscallHandler(SYSCALL_CLONE, HandleSyscallClone);
}

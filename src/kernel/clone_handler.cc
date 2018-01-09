#include "clone_handler.h"

#include "syscall.h"
#include "clone.h"
#include "proc.h"
#include "syscall_handler.h"
#include "page_table.h"

static void HandleSyscallClone(uint64_t interrupt_number,
                               uint64_t options,
                               uint64_t callback,
                               uint64_t new_stack) {
  // TODO security this
  CloneOptions* clone_options = (CloneOptions*)options;
  proc::Clone(clone_options, callback, new_stack);
}

void InitClone() {
  SetSyscallHandler(SYSCALL_CLONE, HandleSyscallClone);
}

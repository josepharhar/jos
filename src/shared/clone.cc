#include "clone.h"

#include "syscall.h"

// TODO add void* arg
pid_t clone(CloneCallback callback, void* new_stack, int flags) {
  SyscallCloneParams clone_options;

  clone_options.copy_page_table = true;
  if (flags & CLONE_VM) {
    clone_options.copy_page_table = false;
  }

  clone_options.copy_fds = false;
  if (flags & CLONE_FILES) {
    clone_options.copy_fds = true;
  }

  clone_options.callback = (uint64_t)callback;
  clone_options.new_stack = (uint64_t)new_stack;
  clone_options.pid_writeback = 0;

  Syscall(SYSCALL_CLONE, (uint64_t)&clone_options);

  return clone_options.pid_writeback;
}

#include "clone.h"

#include "syscall.h"

// TODO add void* arg
void clone(CloneCallback callback, void* new_stack, int flags) {
  CloneOptions clone_options;

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

  Syscall(SYSCALL_CLONE, (uint64_t)&clone_options);
}

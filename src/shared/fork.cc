#include "fork.h"

#include "syscall.h"
#include "getpid.h"

#include "printu.h"

static void asdf() {
  printu("hard coded function getpid: %p\n", Getpid());
  while (1) {}
}

uint64_t Fork() {
  uint64_t parent_pid = Getpid();
  static uint64_t child_pid;
  Syscall(SYSCALL_FORK, (uint64_t) &child_pid, (uint64_t) &asdf); // child_pid only set in parent proc
  uint64_t current_pid = Getpid();

  if (parent_pid != current_pid) {
    // child
    return 0;
  } else {
    // parent
    return child_pid;
  }
}

#include "fcntl.h"

int open(const char* pathname, int flags) {
  SyscallOpenParams params;
  params.filepath = pathname;
  params.flags = flags;
  params.fd_writeback = -2;
  Syscall(SYSCALL_OPEN, (uint64_t)&params);
  return params.fd_writeback;
}

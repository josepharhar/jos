#include "shared/socket.h"

int socket(const char* tcp_address) {
  SyscallSocketParams params;
  params.tcp_address = tcp_address;
  params.status_writeback = -2;
  Syscall(SYSCALL_SOCKET, (uint64_t)&params);
  return params.status_writeback;
}

#include "shared/socket.h"

#include "stdint.h"
#include "syscall_params.h"
#include "syscall.h"

int socket(const char* tcp_address) {
  SyscallSocketParams params;
  params.tcp_address = (char*)tcp_address;
  params.status_writeback = -2;
  Syscall(SYSCALL_SOCKET, (uint64_t)&params);
  return params.status_writeback;
}

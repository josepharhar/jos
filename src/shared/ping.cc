#include "ping.h"

#include "syscall.h"
#include "syscall_params.h"

int ping(char* ip_address) {
  SyscallPingParams params;
  params.ip_addr = ip_address;
  params.status_writeback = -1;
  Syscall(SYSCALL_PING, (uint64_t)&params);
  return params.status_writeback;
}

#include "socket_handler.h"

#include "net.h"
#include "syscall.h"
#include "syscall_params.h"
#include "syscall_handler.h"

namespace net {

static void HandleSyscallSocket(uint64_t interrupt_number,
                                uint64_t param_1,
                                uint64_t param_2,
                                uint64_t param_3) {
  SyscallSocketParams* params = (SyscallSocketParams*)param_1;
}

void InitSocket() {
  SetSyscallHandler(SYSCALL_SOCKET, HandleSyscallSocket);
}

}  // namespace net

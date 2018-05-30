#include "socket_handler.h"

#include "net.h"
#include "syscall.h"
#include "syscall_params.h"
#include "syscall_handler.h"
#include "socket_file.h"

namespace net {

static void HandleSyscallSocket(uint64_t interrupt_number,
                                uint64_t param_1,
                                uint64_t param_2,
                                uint64_t param_3) {
  SyscallSocketParams* params = (SyscallSocketParams*)param_1;

  TcpAddr tcp_addr = TcpAddr::FromString(params->tcp_address);
  if (tcp_addr == TcpAddr::INVALID) {
    params->fd_writeback = -1;
    return;
  }

  SocketFile* socket_file = new SocketFile(tcp_addr);
  ipc::Pipe* socket_pipe = socket_file->Open(ipc::RDWR);
  int fd = proc::AddPipeToProc(proc::GetCurrentProc(), socket_pipe);
  params->fd_writeback = fd;
}

void InitSocket() {
  SetSyscallHandler(SYSCALL_SOCKET, HandleSyscallSocket);
}

}  // namespace net

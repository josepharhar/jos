#include "ping_handler.h"

#include "net.h"
#include "syscall_params.h"

namespace net {

struct PingState {
  proc::BlockedQueue* proc_queue;
  proc::ProcContext* proc;
};

static void PingCallback(void* arg) {
  PingState* state = (PingState*)arg;
  state->proc_queue.UnblockHead();
  delete state;
}

static void HandleSyscallPing(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  SyscallPingParams* params = (SyscallPingParams*)param_1;
  IpAddr ip_addr = IpAddr::FromString(params->ip_addr);

  PingState* state = new PingState();
  state->proc = proc::GetCurrentProc();
  state->proc_queue = proc::BlockedQueue();
  state->proc_queue.BlockCurrentProcNoNesting();

  Ping(ip_addr, PingCallback, state);
}

void InitPing() {
  SetSyscallHandler(SYSCALL_PING, HandleSyscallPing);
}

}  // namespace net

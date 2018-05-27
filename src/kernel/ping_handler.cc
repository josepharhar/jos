#include "ping_handler.h"

#include "net.h"
#include "syscall_params.h"
#include "syscall_handler.h"
#include "syscall.h"
#include "proc.h"
#include "icmp.h"
#include "asm.h"

namespace net {

struct PingState {
  proc::BlockedQueue proc_queue;
  proc::ProcContext* proc;
  SyscallPingParams* params;
};

static void ReceivedPingResponseCallback(void* arg) {
  PingState* state = (PingState*)arg;

  uint64_t old_cr3 = Getcr3();
  bool swap_tables = old_cr3 != state->proc->cr3;
  if (swap_tables) {
    Setcr3(state->proc->cr3);
  }
  state->params->status_writeback = 0;
  if (swap_tables) {
    Setcr3(old_cr3);
  }

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
  state->params = params;

  Ping(ip_addr, ReceivedPingResponseCallback, state);
}

void InitPing() {
  SetSyscallHandler(SYSCALL_PING, HandleSyscallPing);
}

}  // namespace net

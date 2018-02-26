#include "semaphore_handler.h"

#include "semaphore.h"
#include "syscall.h"
#include "syscall_handler.h"
#include "printk.h"
#include "proc.h"

class Semaphore {
 public:
  Semaphore();

 private:
  proc::BlockedQueue proc_queue_;
};

typedef stdj::Map<int, Semaphore*, ((Semaphore*)0)> SemaphoreMap;
static SemaphoreMap semaphore_map;

static void HandleSyscallSemaphore(uint64_t interrupt_number,
                                   uint64_t param_1,
                                   uint64_t param_2,
                                   uint64_t param_3) {
  SyscallSemaphoreRequest* request = (SyscallSemaphoreRequest*)param_1;
  switch (request->type) {
    case SEM_WAIT:
      break;

    case SEM_POST:
      break;

    case SEM_INIT:
      break;

    default:
      printk("bad semaphore request type: %d\n", request->type);
      break;
  }
}

void InitSemaphore() {
  semaphore_map = SemaphoreMap();
  SetSyscallHandler(SYSCALL_SEMAPHORE, HandleSyscallSemaphore);
}

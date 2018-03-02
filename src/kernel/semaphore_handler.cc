#include "semaphore_handler.h"

#include "semaphore.h"
#include "syscall.h"
#include "syscall_handler.h"
#include "printk.h"
#include "proc.h"

struct Semaphore {
  unsigned value;
  proc::BlockedQueue proc_queue;
};

// static SemaphoreMap semaphore_map;

static void HandleSyscallSemaphore(uint64_t interrupt_number,
                                   uint64_t param_1,
                                   uint64_t param_2,
                                   uint64_t param_3) {
  SyscallSemaphoreRequest* request = (SyscallSemaphoreRequest*)param_1;
  sem_t* request_semaphore = request->semaphore;
  // TODO Make sure request_semaphore->name is null terminated
  /*stdj::string semaphore_name(request_semaphore->name);
  printk("HandleSyscallSemaphore semaphore_name: \"%s\"\n",
         semaphore_name.c_str());*/
  int semaphore_number = request_semaphore->semaphore;
  printk("HandleSyscallSemaphore semaphore_number: %d\n", semaphore_number);
  bool is_wait = false;
  proc::ProcContext* current_proc = proc::GetCurrentProc();

  switch (request->type) {
    case SEM_WAIT: {
      if (!current_proc->open_semaphores_.ContainsKey(semaphore_name)) {
        request->status_writeback = -1;
        break;
      }
      break;
    }

    case SEM_POST: {
      if (!current_proc->open_semaphores_.ContainsKey(semaphore_name)) {
        request->status_writeback = -1;
        break;
      }
      break;
    }

    case SEM_INIT: {
      if (!semaphore_map.ContainsKey(semaphore_name)) {
        Semaphore* new_semaphore = new Semaphore();
        semaphore_map.Set(semaphore_name, new_semaphore);
        current_proc->open_semaphores_.Set(semaphore_name, new_semaphore);
      }
      break;
    }

    default:
      printk("bad semaphore request type: %d\n", request->type);
      break;
  }
}

void InitSemaphore() {
  semaphore_map = SemaphoreMap();
  SetSyscallHandler(SYSCALL_SEMAPHORE, HandleSyscallSemaphore);
}

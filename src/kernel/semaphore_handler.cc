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

static SemaphoreMap named_semaphores;

static Semaphore* GetSemaphore(stdj::string name) {
  proc::ProcContext* current_proc = proc::GetCurrentProc();
  if (!current_proc->open_semaphores_.ContainsKey(name)) {
    return 0;
  }
  Semaphore* semaphore = current_proc->open_semaphores_.Get(name);
  if (named_semaphores.Get(name) != semaphore) {
    printk("proc has semaphore open not in named_semaphores!!!\n");
    return 0;
  }
  return semaphore;
}

static void HandleSyscallSemaphore(uint64_t interrupt_number,
                                   uint64_t param_1,
                                   uint64_t param_2,
                                   uint64_t param_3) {
  SyscallSemaphoreRequest* request = (SyscallSemaphoreRequest*)param_1;
  sem_t* request_semaphore = request->semaphore;
  // TODO Make sure request_semaphore->name is null terminated
  stdj::string name(request_semaphore->name);
  printk("HandleSyscallSemaphore name: \"%s\"\n", name.c_str());
  bool is_wait = false;
  proc::ProcContext* current_proc = proc::GetCurrentProc();

  switch (request->type) {
    case SEM_WAIT: {
      Semaphore* semaphore = GetSemaphore(name);
      if (!semaphore) {
        request->status_writeback = -1;
        break;
      }
      break;
    }

    case SEM_POST: {
      Semaphore* semaphore = GetSemaphore(name);
      if (!semaphore) {
        request->status_writeback = -1;
        break;
      }
      break;
    }

    case SEM_OPEN: {
      if (!named_semaphores.ContainsKey(name)) {
        Semaphore* new_semaphore = new Semaphore();
        named_semaphores.Set(name, new_semaphore);
      }
      Semaphore* semaphore = named_semaphores.Get(name);
      if (!current_proc->open_semaphores_.ContainsKey(name)) {
        current_proc->open_semaphores_.Set(name, semaphore);
      }
      request->status_writeback = 0;
      break;
    }

    default:
      printk("bad semaphore request type: %d\n", request->type);
      break;
  }
}

void InitSemaphore() {
  named_semaphores = SemaphoreMap();
  SetSyscallHandler(SYSCALL_SEMAPHORE, HandleSyscallSemaphore);
}

#include "proc.h"

#include "stdint.h"
#include "syscall.h"
#include "syscall_handler.h"
#include "kmalloc.h"
#include "page.h"
#include "asm.h"
#include "printk.h"
#include "linked_list.h"
#include "user.h"
#include "string.h"
#include "frame.h"
#include "ref_counted.h"

#define INTERRUPT_ENABLE_BIT (1 << 9)

// iretq, interrupt stack frame holds:
// ss, rsp, rflags, cs, rip
// https://users.csc.calpoly.edu/~bellardo/courses/2174/454/notes/CPE454-Week02-2.pdf

namespace Proc {

static bool is_proc_running = false;
static uint64_t new_proc_id = 1;

static ProcContext* current_proc = 0;
static ProcContext* next_proc = 0; // set by ProcReschedule()
static ProcContext* main_proc = 0; // context of thread that called ProcRun()

static LinkedList<ProcContext*>* proc_list = nullptr;


static void AssertRunning() {
  DCHECK(proc_list);
  DCHECK(is_proc_running);
}



ProcContext* GetCurrentProc() {
  return current_proc;
}






// called once during kernel initialization
void Init() {
  SetSyscallHandler(SYSCALL_YIELD, HandleSyscallYield);
  SetSyscallHandler(SYSCALL_EXIT, HandleSyscallExit);
  SetSyscallHandler(SYSCALL_PROC_RUN, HandleSyscallProcRun);
  proc_list = new LinkedList<ProcContext*>();
}

// starts system, returns when all threads are complete
void Run() {
  if (proc_list->IsEmpty()) {
    // there are no procs to run
    printk("Start() proc_list.IsEmpty(): %p\n", proc_list->IsEmpty());
    return;
  }

  current_proc = proc_list->Get(0); // current_proc will be run first
  next_proc = 0;
  Syscall(SYSCALL_PROC_RUN);
}

ProcContext* CreateKthread(KthreadFunction entry_point, void* arg) {
  ProcContext* new_proc = new ProcContext();
  new_proc->rip = (uint64_t) entry_point;
  new_proc->cs = 0x8; // kernel or user, for privilege level
  new_proc->rsp = (uint64_t) StackAllocate(); // TODO consider stack overflow, its only 2MB virt
                                              // TODO free this when changing proc to user?
  new_proc->ss = 0; // for kernel
  new_proc->rflags = INTERRUPT_ENABLE_BIT;
  new_proc->pid = new_proc_id++;
  new_proc->cr3 = (uint64_t) Getcr3();
  new_proc->page_table = new PageTable(new_proc->cr3); // TODO make these refcounted?

  // set first C argument to new proc function to void* arg
  new_proc->rdi = (uint64_t) arg;

  // push ProcExit() onto stack
  uint64_t* stack_pointer = (uint64_t*) new_proc->rsp;
  *stack_pointer = (uint64_t) &ProcExit;

  // add new_proc to linked list
  proc_list->Add(new_proc);

  return new_proc;
}

// this is intended for user processes for clone()
ProcContext* Clone(CloneOptions* clone_options, uint64_t new_rip, uint64_t new_stack) {
  AssertRunning();

  SaveState(current_proc); // update current_proc registers
  ProcContext* new_proc = new ProcContext(*current_proc);

  // TODO create more clone() settings to set new proc's registers?

  // TODO consolidate start_at_callback and new_rip
  if (clone_options->start_at_callback) {
    new_proc->rip = new_rip;
  }

  if (new_stack) {
    new_proc->rsp = new_stack;
    new_proc->rbp = new_stack;
  }

  if (clone_options->copy_page_table) {
    new_proc->page_table = current_proc->page_table->Clone();
    new_proc->cr3 = new_proc->page_table->p4_entry();
  }

  new_proc->pid = new_proc_id++;

  proc_list->Add(new_proc);

  return new_proc;
}

// returns 0 if all procs are blocked
// uses round robin from current_proc
static ProcContext* GetNextUnblockedProc() {
  if (!current_proc || proc_list->IsEmpty()) {
    printk("GetNextUnblockedProc() current_proc: %p, proc_list->IsEmpty(): %p\n", current_proc, proc_list->IsEmpty());
    return 0;
  }

  BEGIN_CS(); // interrupts can change blocking

  ProcContext* proc = current_proc;

  do {
    proc = proc_list->GetNext(proc);

    if (!proc->is_blocked) {
      END_CS();
      return proc;
    }

    if (proc == current_proc) {
      // we have looped all the way around and have found no unblocked procs
      END_CS();
      return 0;
    }
  } while (proc != current_proc);

  END_CS();
  return 0;
}

// sets next_proc to the next process to switch contexts to during yield or exit
// if there is only one proc left, then sets next_proc = current_proc
void Reschedule() {
  if (!current_proc || proc_list->IsEmpty()) {
    // there are no processes to schedule. this shouldn't happen, right?
    printk("ProcReschedule() current_proc: %p, proc_list->IsEmpty(): %p\n", current_proc, proc_list->IsEmpty());
    return;
  }

  int interrupts_were_enabled = are_interrupts_enabled();

  do {
    sti(); // force disable interrupts
    // hope that interrupts are handled in between these lines.
    // try to find an unblocked proc multiple times to
    // reduce chance of interrupt handling before halting
    //for (int i = 0; !next_proc && i < 30; i++) {
    for (int i = 0; i < 30; i++) {
      next_proc = GetNextUnblockedProc();
    }
    // TODO reduce number of instructions between block checking and hlt()
    if (!next_proc) {
      hlt();
    }
  } while (!next_proc);

  if (interrupts_were_enabled) {
    cli();
  }
}

void Yield() {
  Reschedule();
  Syscall(SYSCALL_YIELD);
}

void Exit() {
  Reschedule();
  Syscall(SYSCALL_EXIT);
}

extern uint64_t stack_save_state_address[];
// TODO make static?
uint64_t* GetStackSaveState() {
  return (uint64_t*) stack_save_state_address[0];
}

// TODO make static?
void SaveState(struct ProcContext* proc) {
  uint64_t* stack_save_state = GetStackSaveState();
  proc->rbp = stack_save_state[1];
  proc->r15 = stack_save_state[2];
  proc->r14 = stack_save_state[3];
  proc->r13 = stack_save_state[4];
  proc->r12 = stack_save_state[5];
  proc->r11 = stack_save_state[6];
  proc->r10 = stack_save_state[7];
  proc->r9 = stack_save_state[8];
  proc->r8 = stack_save_state[9];
  proc->rsi = stack_save_state[10];
  proc->rdx = stack_save_state[11];
  proc->rcx = stack_save_state[12];
  proc->rbx = stack_save_state[13];
  proc->rax = stack_save_state[14];
  proc->rdi = stack_save_state[15];
  proc->rip = stack_save_state[16];
  proc->cs = stack_save_state[17];
  proc->rflags = stack_save_state[18];
  proc->rsp = stack_save_state[19];
  proc->ss = stack_save_state[20];
  proc->cr3 = (uint64_t) Getcr3();
}

// TODO make static
void RestoreState(struct ProcContext* proc) {
  //printk("RestoreState() restoring rip %p\n", proc->rip);
  uint64_t* stack_save_state = GetStackSaveState();
  //printk("RestoreState() stack_save_state: %p\n", stack_save_state);
  stack_save_state[1] = proc->rbp;
  stack_save_state[2] = proc->r15;
  stack_save_state[3] = proc->r14;
  stack_save_state[4] = proc->r13;
  stack_save_state[5] = proc->r12;
  stack_save_state[6] = proc->r11;
  stack_save_state[7] = proc->r10;
  stack_save_state[8] = proc->r9;
  stack_save_state[9] = proc->r8;
  stack_save_state[10] = proc->rsi;
  stack_save_state[11] = proc->rdx;
  stack_save_state[12] = proc->rcx;
  stack_save_state[13] = proc->rbx;
  stack_save_state[14] = proc->rax;
  stack_save_state[15] = proc->rdi;
  stack_save_state[16] = proc->rip;
  stack_save_state[17] = proc->cs;
  stack_save_state[18] = proc->rflags;
  stack_save_state[19] = proc->rsp;
  stack_save_state[20] = proc->ss;
  Setcr3(proc->cr3);
}

static void HandleSyscallProcRun(uint64_t syscall_number, uint64_t param_1, uint64_t param_2, uint64_t param_3) {
  // save "real" state into main_proc, and load first proc in current_proc
  if (!current_proc) {
    printk("HandleSyscallProcRun() current_proc: %p\n", current_proc);
    return;
  }
  printk("HandleSyscallProcRun\n");

  is_proc_running = 1;

  //main_proc = (ProcContext*) kcalloc(sizeof(ProcContext));
  main_proc = new ProcContext();
  SaveState(main_proc);
  RestoreState(current_proc);
}

static void HandleSyscallYield(uint64_t syscall_number, uint64_t param_1, uint64_t param_2, uint64_t param_3) {
  //TODO investigate more   printk("HandleSyscallYield() are_interrupts_enabled: %d\n", are_interrupts_enabled());
  // switch contexts
  if (!current_proc || !next_proc) {
    printk("HandleSyscallYield() current_proc: %p, next_proc: %p\n", current_proc, next_proc);
    return;
  }

  // save context put on stack by irq_syscall
  SaveState(current_proc);

  current_proc = next_proc;
  next_proc = 0;

  // put new context onto stack, to be restored by irq_syscall
  RestoreState(current_proc);
}

// TODO make this get handled on a different stack to stop the GP faults?
static void HandleSyscallExit(uint64_t syscall_number, uint64_t param_1, uint64_t param_2, uint64_t param_3) {
  // destroy current process and switch context to next process
  
  if (!current_proc || !next_proc || proc_list->IsEmpty()) {
    printk("HandleSyscallExit() current_proc: %p, next_proc: %p, proc_list->IsEmpty(): %p\n",
        current_proc, next_proc, proc_list->IsEmpty());
    return;
  }

  // TODO in order to round robin correctly,
  // set current_proc to the proc prior to current_proc in the list
  // remove current_proc
  // call ProcReschedule()
  // current_proc = next_proc
  // restore current_proc

  struct ProcContext* current_proc_prev = proc_list->GetPrevious(current_proc);

  // remove current_proc from linked list
  proc_list->Remove(current_proc);

  // free current_proc resources
  // TODO free page table
  StackFree((void*) current_proc->rsp); // StackFree() takes any address within the stack
  kfree(current_proc);

  // set current_proc to the proc prior to current_proc in the list
  current_proc = current_proc_prev;

  if (proc_list->IsEmpty()) {
    // no more procs to run, restore main context
    is_proc_running = 0;
    RestoreState(main_proc);
    kfree(main_proc);
  } else {
    ProcReschedule();
    current_proc = next_proc;
    next_proc = 0;
    RestoreState(current_proc);
  }
}

// TODO delet this
// Initializes a ProcQueue structure (mainly sets head to NULL).
// Called once for each ProcQueue during driver initialization
/*void ProcInitQueue(struct ProcQueue* queue) {
  queue->head = 0;
}*/

// Unblocks one process from the ProcQueue,
// moving it back to the scheduler.
// Called by interrupt handler?
// Returns whether or not a proc was unblocked
int UnblockHead(ProcQueue* queue) {
  BEGIN_CS();

  int removed_proc = 0;
  struct ProcContext* unblocked = queue->head;
  if (unblocked) {
    queue->head = unblocked->blocked_next;
    unblocked->blocked_next = 0;
    unblocked->is_blocked = 0;
    removed_proc = 1;
  } else {
    // this queue is empty
  }

  END_CS();
  return removed_proc;
}

// Unblocks all processes from the ProcQueue,
// moving them all back to the scheduler
void UnblockAll(ProcQueue* queue) {
  BEGIN_CS();
  while (UnblockHead(queue));
  END_CS();
}

// Blocks the current process.
// Called by system call handler.
//void ProcBlockOn(struct ProcQueue* queue, int enable_ints) {
void BlockOn(ProcQueue* queue) {
  BEGIN_CS(); // appending to the queue must be atomic, it can be edited by interrupt handlers
  /*int interrupts_were_enabled = are_interrupts_enabled();
  cli();*/

  if (queue->head) {
    struct ProcContext* last_in_queue = queue->head;
    while (last_in_queue->blocked_next) {
      last_in_queue = last_in_queue->blocked_next;
    }
    last_in_queue->blocked_next = current_proc;
  } else {
    queue->head = current_proc;
  }

  current_proc->is_blocked = 1;

  END_CS();

  ProcYield(); // i can do nested syscalls, right?
}

void ProcPrint() {
  if (proc_list->IsEmpty()) {
    printk("ProcPrint() no processes\n");
  } else {
    ProcContext* proc = proc_list->Get(0);
    do {
      printk("pid %d is_blocked %d\n", proc->pid, proc->is_blocked);
      proc = proc_list->GetNext(proc);
    } while (proc != proc_list->Get(0));
  }
}

int IsRunning() {
  return is_proc_running;
}

bool IsKernel() {
  uint64_t privilege_level = (current_proc->rflags >> 12) & 3;
  return privilege_level == 0; // zero is kernel, 3 is user
}

}  // namespace Proc

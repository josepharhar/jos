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
#include "jarray.h"
#include "irq.h"

#define INTERRUPT_ENABLE_BIT (1 << 9)

// iretq, interrupt stack frame holds:
// ss, rsp, rflags, cs, rip
// https://users.csc.calpoly.edu/~bellardo/courses/2174/454/notes/CPE454-Week02-2.pdf

// stack save state from irq_nasm
extern uint64_t stack_save_state_address[];

namespace proc {

static bool is_proc_running = false;
static uint64_t new_proc_id = 1;

static ProcContext* current_proc = 0;
static ProcContext* next_proc = 0;  // set by Reschedule()
static ProcContext* main_proc = 0;  // context of thread that called ProcRun()

static stdj::Array<ProcContext*>* proc_list = nullptr;

// TODO use this more?
static void AssertRunning() {
  DCHECK(proc_list);
  DCHECK(is_proc_running);
}

ProcContext* GetCurrentProc() {
  return current_proc;
}

extern "C" {
static void HandleSyscallYield(uint64_t syscall_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3);
static void HandleSyscallExit(uint64_t syscall_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3);
static void HandleSyscallProcRun(uint64_t syscall_number,
                                 uint64_t param_1,
                                 uint64_t param_2,
                                 uint64_t param_3);
}

// called once during kernel initialization
void Init() {
  SetSyscallHandler(SYSCALL_YIELD, HandleSyscallYield);
  SetSyscallHandler(SYSCALL_EXIT, HandleSyscallExit);
  SetSyscallHandler(SYSCALL_PROC_RUN, HandleSyscallProcRun);
  proc_list = new stdj::Array<ProcContext*>();
}

// starts system, returns when all threads are complete
void Run() {
  if (proc_list->IsEmpty()) {
    // there are no procs to run
    printk("Start() proc_list.IsEmpty(): %p\n", proc_list->IsEmpty());
    return;
  }

  current_proc = proc_list->Get(0);  // current_proc will be run first
  next_proc = 0;
  Syscall(SYSCALL_PROC_RUN);
}

ProcContext* CreateKthread(KthreadFunction entry_point, void* arg) {
  ProcContext* new_proc = new ProcContext();
  new_proc->rip = (uint64_t)entry_point;
  new_proc->cs = 0x8;  // kernel or user, for privilege level

  // TODO consider stack overflow, its only 2MB virt
  // TODO free this when changing proc to user?
  new_proc->rsp = (uint64_t)StackAllocate();

  new_proc->ss = 0;  // for kernel
  new_proc->rflags = INTERRUPT_ENABLE_BIT;
  new_proc->pid = new_proc_id++;
  new_proc->cr3 = (uint64_t)Getcr3();

  // set first C argument to new proc function to void* arg
  new_proc->rdi = (uint64_t)arg;

  // push Exit() onto stack
  uint64_t* stack_pointer = (uint64_t*)new_proc->rsp;
  *stack_pointer = (uint64_t)&Exit;

  // add new_proc to linked list
  proc_list->Add(new_proc);

  return new_proc;
}

// this is intended for user processes for clone()
ProcContext* Clone(SyscallCloneParams* clone_options) {
  // TODO use this more in other functions
  AssertRunning();

  ProcContext* new_proc = new ProcContext(*current_proc);

  // TODO create more clone() settings to set new proc's registers?

  if (clone_options->callback) {
    new_proc->rip = clone_options->callback;
  } else {
    new_proc->rip = current_proc->rip;
  }

  // set stack registers
  if (clone_options->new_stack) {
    new_proc->rsp = clone_options->new_stack;
    new_proc->rbp = clone_options->new_stack;
  }

  // copy page table
  if (clone_options->copy_page_table) {
    new_proc->cr3 = page::CopyPageTable(current_proc->cr3);
  }

  // set new pid
  new_proc->pid = new_proc_id++;
  printk("new_proc->pid: %d, new_proc->rip: %p\n", new_proc->pid,
         new_proc->rip);
  printk("old_proc->cr3: %p, new_proc->cr3: %p\n", current_proc->cr3,
         new_proc->cr3);

  // copy file descriptors
  new_proc->fd_map_ = FdMap();
  if (clone_options->copy_fds) {
    FdMap* new_fd_map = &new_proc->fd_map_;
    FdMap* old_fd_map = &current_proc->fd_map_;
    for (int i = 0; i < old_fd_map->Size(); i++) {
      ipc::Pipe* pipe_to_copy = old_fd_map->GetAt(i);
      ipc::File* file = pipe_to_copy->GetFile();
      ipc::Pipe* new_pipe = file->Open(pipe_to_copy->GetMode());
      int fd = old_fd_map->GetKeyAt(i);
      new_fd_map->Set(fd, new_pipe);
    }
  }

  // add new proc to the master list of procs
  proc_list->Add(new_proc);

  clone_options->pid_writeback = new_proc->pid;

  return new_proc;
}

// returns 0 if all procs are blocked
// uses round robin from current_proc
static ProcContext* GetNextUnblockedProc() {
  if (!current_proc || proc_list->IsEmpty()) {
    printk(
        "GetNextUnblockedProc() current_proc: %p, proc_list->IsEmpty(): %p\n",
        current_proc, proc_list->IsEmpty());
    return 0;
  }

  BEGIN_CS();  // interrupts can change blocking

  ProcContext* proc = current_proc;

  do {
    proc = proc_list->GetNextValue(proc);

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
    printk("Reschedule() current_proc: %p, proc_list->IsEmpty(): %p\n",
           current_proc, proc_list->IsEmpty());
    return;
  }

  int interrupts_were_enabled = are_interrupts_enabled();

  do {
    sti();  // force disable interrupts
    // hope that interrupts are handled in between these lines.
    // try to find an unblocked proc multiple times to
    // reduce chance of interrupt handling before halting
    // for (int i = 0; !next_proc && i < 30; i++) {
    for (int i = 0; i < 30; i++) {
      next_proc = GetNextUnblockedProc();
    }
    // TODO reduce number of instructions between block checking and hlt()
    if (!next_proc) {
      asm volatile("hlt");
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

// TODO make static?
uint64_t* GetStackSaveState() {
  return (uint64_t*)stack_save_state_address[0];
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
  proc->cr3 = (uint64_t)Getcr3();
}

// TODO make static
void RestoreState(struct ProcContext* proc) {
  uint64_t* stack_save_state = GetStackSaveState();
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

static void HandleSyscallProcRun(uint64_t syscall_number,
                                 uint64_t param_1,
                                 uint64_t param_2,
                                 uint64_t param_3) {
  if (!current_proc || is_proc_running) {
    printk("HandleSyscallProcRun() current_proc: %p is_proc_running: %d\n",
           current_proc, is_proc_running);
    return;
  }

  is_proc_running = true;

  // save "real" state into main_proc, and load first proc in current_proc
  main_proc = new ProcContext();
  SaveState(main_proc);
  RestoreState(current_proc);
}

static void HandleSyscallYield(uint64_t syscall_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3) {
  // switch contexts
  if (!current_proc || !next_proc) {
    printk("HandleSyscallYield() current_proc: %p, next_proc: %p\n",
           current_proc, next_proc);
    return;
  }

  /*printk("HandleSyscallYield\n");
  printk("  current pid: %d, rip: %p\n", current_proc->pid, current_proc->rip);
  printk("     next pid: %d, rip: %p\n", next_proc->pid, next_proc->rip);*/
  current_proc = next_proc;
  next_proc = 0;

  // put new context onto stack, to be restored by irq_syscall
  RestoreState(current_proc);
}

// TODO make this get handled on a different stack to stop the GP faults?
static void HandleSyscallExit(uint64_t syscall_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  // destroy current process and switch context to next process

  if (!current_proc || !next_proc || proc_list->IsEmpty()) {
    printk(
        "HandleSyscallExit() current_proc: %p, next_proc: %p, "
        "proc_list->IsEmpty(): %p\n",
        current_proc, next_proc, proc_list->IsEmpty());
    return;
  }

  // TODO in order to round robin correctly,
  // set current_proc to the proc prior to current_proc in the list
  // remove current_proc
  // call Reschedule()
  // current_proc = next_proc
  // restore current_proc

  ProcContext* current_proc_prev = proc_list->GetPreviousValue(current_proc);

  // remove current_proc from linked list
  proc_list->RemoveAt(proc_list->GetIndexOfValue(current_proc));

  // free current_proc resources
  // TODO free page table
  // StackFree() takes any address within the stack
  StackFree((void*)current_proc->rsp);
  kfree(current_proc);

  // set current_proc to the proc prior to current_proc in the list
  current_proc = current_proc_prev;

  if (proc_list->IsEmpty()) {
    // no more procs to run, restore main context
    is_proc_running = 0;
    RestoreState(main_proc);
    kfree(main_proc);
  } else {
    Reschedule();
    current_proc = next_proc;
    next_proc = 0;
    RestoreState(current_proc);
  }
}

BlockedQueue::BlockedQueue() {}
BlockedQueue::~BlockedQueue() {}

// Unblocks one process from the ProcQueue,
// moving it back to the scheduler.
// Called by interrupt handler?
// Returns unblocked proc
ProcContext* BlockedQueue::UnblockHead() {
  BEGIN_CS();

  ProcContext* removed_proc = 0;
  if (!queue_.IsEmpty()) {
    removed_proc = queue_.Remove();
    removed_proc->is_blocked = 0;
  }

  END_CS();
  return removed_proc;
}

// Unblocks all processes from the ProcQueue,
// moving them all back to the scheduler
void BlockedQueue::UnblockAll() {
  BEGIN_CS();
  while (UnblockHead())
    ;
  END_CS();
}

// Blocks the current process.
// Called by system call handler.
// void ProcBlockOn(struct ProcQueue* queue, int enable_ints) {
void BlockedQueue::BlockCurrentProc() {
  // appending to the queue must be atomic, it can be edited by interrupt
  // handlers
  BEGIN_CS();
  queue_.Add(current_proc);
  current_proc->is_blocked = 1;
  END_CS();

  Yield();  // i can do nested syscalls, right?
}

void BlockedQueue::BlockCurrentProcNoNesting() {
  // appending to the queue must be atomic, it can be edited by interrupt
  // handlers
  BEGIN_CS();
  queue_.Add(current_proc);
  current_proc->is_blocked = (int)GetLastSyscallNum();
  printk("BlockCurrentProcNoNesting() pid: %d, is_blocked: %d\n",
         current_proc->pid, current_proc->is_blocked);
  END_CS();

  // formerly YieldNoNesting()
  /*Reschedule();
  current_proc = next_proc;
  next_proc = 0;
  RestoreState(current_proc);*/
}

int BlockedQueue::Size() {
  return queue_.Size();
}

void Print() {
  // printk("proc::Print() printing procs:\n");
  for (uint64_t i = 0; i < proc_list->Size(); i++) {
    ProcContext* proc = proc_list->Get(i);
    printk("  pid: %d, blk: %d, rip: %p, cr3: %p\n", proc->pid,
           proc->is_blocked, proc->rip, proc->cr3);
  }
}

bool IsRunning() {
  return is_proc_running;
}

bool IsKernel() {
  uint64_t privilege_level = (current_proc->rflags >> 12) & 3;
  return privilege_level == 0;  // zero is kernel, 3 is user
}

// Loads a program from memory to replace the current process's program
void ExecCurrentProc(ELFInfo elf_info, uint8_t* file_data) {
  // TODO create a new page table for this process
  //   and recursively free the current one and its frames in user space
  // TODO blow away/sanitize current_proc's registers
  // TODO allocate user stack
  // TODO set to user mode? only if already user or supposed to be user?
  // TODO sanitize user proc's input

  // allocate user stack
  // printk("allocating user stack from %p to %p\n", USER_STACK_TOP,
  // USER_STACK_BOTTOM);
  AllocateUserSpace(USER_STACK_TOP, USER_STACK_SIZE);
  uint64_t user_stack_bottom = USER_STACK_BOTTOM - 512 - 4096;  // TODO
  // put Exit() on stack
  // TODO Exit() location must come from user executable, not this one
  uint64_t* stack_pointer = (uint64_t*)user_stack_bottom;
  /*printk("putting Exit on stack at %p\n", user_stack_bottom);
  *stack_pointer = (uint64_t) &Exit;*/

  // allocate and fill user text/data
  printk("ExecCurrentProc() AllocateUserSpace()\n");
  AllocateUserSpace(elf_info.load_address, elf_info.num_bytes);
  printk("ExecCurrentProc() memcpy()\n");
  // memset((void*)USER_STACK_BOTTOM - (4096 * 4), 0, 4096 * 4 + 1);
  uint64_t* stack = (uint64_t*)0x000007FFFFFFF000;
  printk("GetPhysicalAddress(%p): %p\n", stack,
         page::GetPhysicalAddress(current_proc->cr3, (uint64_t)stack));
  PageTableEntry ptentry = GetP1Entry((uint64_t)stack, DO_NOT_CREATE_ENTRIES);
  printk("        GetP1Entry(%p): %p\n", GetP1Entry((uint64_t)stack,
        DO_NOT_CREATE_ENTRIES)->);
  *stack = 4880;
  printk("GetPhysicalAddress(%p): %p\n", stack,
         page::GetPhysicalAddress(current_proc->cr3, (uint64_t)stack));
  memcpy((void*)elf_info.load_address, file_data + elf_info.file_offset,
         elf_info.file_size);

  current_proc->rip = elf_info.instruction_pointer;
  // current_proc->rflags |= (3 << 12);
  // TODO
  current_proc->cs = GDT_USER_CS + 3;
  current_proc->ss = GDT_USER_DS + 3;
  // current_proc->rsp = user_stack_bottom;

  RestoreState(current_proc);
}

uint64_t GetCurrentPid() {
  return current_proc->pid;
}

void SaveStateToCurrentProc() {
  if (!IsRunning()) {
    return;
  }

  ProcContext fake_proc;
  SaveState(&fake_proc);
  if (fake_proc.rip == 0) {
    // TODO sometimes garbage data gets in from SaveState(). WHY???
    //   This check is hacky and should not be necessary.
    //   Is this happening because the kernel is getting interrupted?
    printk("this should not happen - save state has rip = 0\n");
    return;
  }

  // TODO this is very hacky and this check should have been covered
  //   by PreInterrupt() checking interrupt_context
  if (fake_proc.rip < 0xF000000 && current_proc->rip > 0xF000000) {
    /*printk("PROC rip TO KERNEL SPACE! %p -> %p\n", current_proc->rip,
    fake_proc.rip);
    printk("  context: %d, syscall: %d, interrupt: %d\n",
        GetInterruptContext(), GetLastSyscallNum(), GetLastInterruptNumber());
    while (1) {
      asm volatile ("hlt");
    }*/
    return;
  }

  SaveState(current_proc);
}

void ProcContext::PrintValues() {
  printk("Printing ProcContext values:\n");
  printk("   rbp: %p\n", rbp);
  printk("   rip: %p\n", rip);
  printk("    cs: %p\n", cs);
  printk("rflags: %p\n", rflags);
  printk("   rsp: %p\n", rsp);
  printk("    ss: %p\n", ss);
}

int ProcContext::GetNewFd() {
  for (int i = 0; i < MAX_FDS; i++) {
    if (!fd_map_.ContainsKey(i)) {
      return i;
    }
  }
  return -1;
}

// Returns new fd allocated for current proc
int AddPipeToCurrentProc(ipc::Pipe* pipe) {
  int new_fd = current_proc->GetNewFd();
  if (new_fd == -1) {
    // ran out of fds
    return new_fd;
  }
  printk("pid %d installing fd %d -> %p\n", current_proc->pid, new_fd, pipe);
  current_proc->fd_map_.Set(new_fd, pipe);
  return new_fd;
}

ipc::Pipe* GetPipeForFdFromCurrentProc(int fd) {
  return current_proc->fd_map_.Get(fd);
}

static ProcContext* FindUnblockedProc() {
  ProcContext* proc = current_proc;
  do {
    proc = proc_list->GetNextValue(proc);
    if (!proc->is_blocked) {
      return proc;
    }
  } while (proc != current_proc);
  return 0;
}

void PreemptProc() {
  if (!IsRunning()) {
    return;
  }

  if (proc_list->Size() < 2) {
    return;
  }

  ProcContext* next_unblocked_proc = FindUnblockedProc();
  if (next_unblocked_proc && next_unblocked_proc != current_proc) {
    printk("PreemptProc() switching procs pid %d -> %d\n", current_proc->pid,
           next_unblocked_proc->pid);
    Print();
    current_proc = next_unblocked_proc;
    RestoreState(current_proc);
  }
}

void EndOfSyscallReschedule() {
  if (!current_proc->is_blocked) {
    return;
  }

  InterruptContextType interrupt_context = GetInterruptContext();
  uint64_t last_syscall_num = GetLastSyscallNum();

  ProcContext* unblocked_proc = 0;
  while (!unblocked_proc) {
    unblocked_proc = FindUnblockedProc();
    if (!unblocked_proc) {
      sti();
      asm volatile("hlt");
      cli();
    }
  }

  if (current_proc != unblocked_proc) {
    printk("EndOfSyscallReschedule() switching procs pid %d -> %d\n",
           current_proc->pid, unblocked_proc->pid);
    printk("  syscall: %d\n", GetLastSyscallNum());

    printk("  pid %d rbp->phys: %p -> %p\n", current_proc->pid,
           current_proc->rbp,
           page::GetPhysicalAddress(current_proc->cr3, current_proc->rbp));
    printk("  pid %d rbp->phys: %p -> %p\n", unblocked_proc->pid,
           unblocked_proc->rbp,
           page::GetPhysicalAddress(unblocked_proc->cr3, unblocked_proc->rbp));

    Print();
    current_proc = unblocked_proc;
    RestoreState(current_proc);
  }
}

}  // namespace proc

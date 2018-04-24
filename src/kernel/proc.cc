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
  new_proc->bottom_of_stack = page::StackAllocate();
  new_proc->rsp = new_proc->bottom_of_stack;
  // new_proc->rbp = new_proc->rsp; // TODO uncomment this

  new_proc->ss = 0;  // for kernel
  new_proc->rflags = INTERRUPT_ENABLE_BIT;
  new_proc->pid = new_proc_id++;
  new_proc->cr3 = Getcr3();

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

  new_proc->parent = current_proc;

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

// TODO delet this
void Yield() {
  Reschedule();
  Syscall(SYSCALL_YIELD);
}

// TODO delet this
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
  proc->cr3 = Getcr3();
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

  current_proc = next_proc;
  next_proc = 0;

  // put new context onto stack, to be restored by irq_syscall
  RestoreState(current_proc);
}

void TryFinishWaiting(ProcContext* proc) {
  if (!proc || !proc->wait_for_zombie || !proc->wait_params ||
      !proc->zombie_queue.Size()) {
    return;
  }

  ZombieContext zombie = proc->zombie_queue.RemoveAt(0);

  proc->wait_for_zombie->UnblockHead();
  delete proc->wait_for_zombie;
  proc->wait_for_zombie = 0;

  uint64_t old_cr3 = Getcr3();
  bool switch_tables = old_cr3 != proc->cr3;
  if (switch_tables) {
    Setcr3(proc->cr3);
  }
  *(proc->wait_params->exit_status_writeback) = zombie.exit_status;
  proc->wait_params->pid_writeback = zombie.proc->pid;
  if (switch_tables) {
    Setcr3(old_cr3);
  }
  proc->wait_params = 0;

  delete zombie.proc;
}

// TODO make this get handled on a different stack to stop the GP faults?
static void HandleSyscallExit(uint64_t syscall_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  if (!current_proc || proc_list->IsEmpty()) {
    printk("HandleSyscallExit() procs not running!\n");
    return;
  }

  SyscallExitParams* params = (SyscallExitParams*)param_1;

  ProcContext* proc_to_delete = current_proc;

  if (proc_to_delete->parent) {
    ZombieContext new_zombie;
    new_zombie.proc = proc_to_delete;
    new_zombie.exit_status = params->exit_status;

    proc_to_delete->parent->zombie_queue.Add(new_zombie);
    for (int i = 0; i < proc_to_delete->zombie_queue.Size(); i++) {
      proc_to_delete->parent->zombie_queue.Add(
          proc_to_delete->zombie_queue.Get(i));
    }
    TryFinishWaiting(proc_to_delete->parent);
  }

  current_proc = proc_list->GetPreviousValue(current_proc);
  proc_list->RemoveValue(proc_to_delete);
  // TODO free page table
  // TODO free fd resources
  // kfree(proc_to_delete);

  if (proc_list->IsEmpty()) {
    printk("  no procs left, returning to kernel\n");
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
void BlockedQueue::BlockCurrentProcNoNesting() {
  // appending to the queue must be atomic, it can be edited by interrupt
  // handlers
  BEGIN_CS();
  queue_.Add(current_proc);
  current_proc->is_blocked = (int)GetLastSyscallNum();
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

void ExecProc(ProcContext* proc,
              ELFInfo elf_info,
              uint8_t* file_data,
              char** argv_src) {
  uint64_t old_cr3 = Getcr3();
  bool switch_tables = proc->cr3 != old_cr3;
  if (switch_tables) {
    Setcr3(proc->cr3);
  }

  // TODO create a new page table for this process
  //   and recursively free the current one and its frames in user space
  // TODO blow away/sanitize proc's registers
  // TODO allocate user stack
  // TODO set to user mode? only if already user or supposed to be user?
  // TODO sanitize user proc's input

  // TODO formalize the use of this address?
  //   get it from the normal user heap instead?
  //   remotely access the user heap allocator?
  static const uint64_t argv_addr = 0x00000A0000000000;

  int argv_size;
  uint64_t total_string_length = 0;
  for (argv_size = 0; argv_src[argv_size]; argv_size++) {
    total_string_length += strlen(argv_src[argv_size]) + 1;
  }
  total_string_length += (argv_size + 1) * sizeof(uint64_t);
  page::AllocateUserSpace(proc->cr3, argv_addr, total_string_length);

  char** argv_dest = (char**)argv_addr;
  memset(argv_dest, 0, (argv_size + 1) * sizeof(uint64_t));
  char* argv_string_dest = (char*)argv_addr;
  argv_string_dest += (argv_size + 1) * sizeof(uint64_t);
  for (int i = 0; i < argv_size; i++) {
    strcpy(argv_string_dest, argv_src[i]);
    argv_dest[i] = argv_string_dest;

    argv_string_dest += strlen(argv_src[i]);
    argv_string_dest[i] = 0;
    argv_string_dest++;
  }

  proc->rdi = (uint64_t)argv_size;
  proc->rsi = (uint64_t)argv_dest;

  // allocate user stack
  page::AllocateUserSpace(proc->cr3, USER_STACK_TOP, USER_STACK_SIZE);
  proc->rsp = USER_STACK_BOTTOM;
  proc->rbp = USER_STACK_BOTTOM;
  // TODO StackFree(proc->bottom_of_stack);
  proc->bottom_of_stack = USER_STACK_BOTTOM;

  // allocate and fill user text/data
  page::AllocateUserSpace(proc->cr3, elf_info.load_address, elf_info.num_bytes);
  memcpy((void*)elf_info.load_address, file_data + elf_info.file_offset,
         elf_info.file_size);

  proc->rip = elf_info.instruction_pointer;
  // proc->rflags |= (3 << 12);
  // TODO
  proc->cs = GDT_USER_CS + DPL_USER;
  proc->ss = GDT_USER_DS + DPL_USER;

  // TODO write sbrk() and make a real user memroy allocator
  page::AllocateUserSpace(proc->cr3, 0x0000090000000000, 32768);

  if (switch_tables) {
    Setcr3(old_cr3);
  }
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
  // TODO HandlePageFault() recursively triggers a page fault
  //   by reaching this code because current_proc is not backed by
  //   a physical frame yet!!!
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
int AddPipeToProc(ProcContext* proc, ipc::Pipe* pipe) {
  int new_fd = proc->GetNewFd();
  if (new_fd == -1) {
    // ran out of fds
    return new_fd;
  }
  proc->fd_map_.Set(new_fd, pipe);
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
    // Print();
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
    /*printk("EndOfSyscallReschedule() switching procs pid %d -> %d\n",
           current_proc->pid, unblocked_proc->pid);
    printk("  syscall: %d\n", GetLastSyscallNum());

    printk("  pid %d rbp->phys: %p -> %p\n", current_proc->pid,
           current_proc->rbp,
           page::GetPhysicalAddress(current_proc->cr3, current_proc->rbp));
    printk("  pid %d rbp->phys: %p -> %p\n", unblocked_proc->pid,
           unblocked_proc->rbp,
           page::GetPhysicalAddress(unblocked_proc->cr3, unblocked_proc->rbp));
    Print();*/
    current_proc = unblocked_proc;
    RestoreState(current_proc);
  }
}

}  // namespace proc

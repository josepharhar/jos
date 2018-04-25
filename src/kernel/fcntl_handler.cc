#include "fcntl_handler.h"

#include "syscall.h"
#include "syscall_handler.h"
#include "fcntl.h"
#include "proc.h"
#include "kernel/vfs/find_file.h"
#include "kernel/vfs/globals.h"
#include "ipc.h"
#include "asm.h"
#include "disk_file.h"

struct OpenContext {
  SyscallOpenParams* params;
  proc::BlockedQueue blocked_queue;
  proc::ProcContext* proc;
};

static void FindFileCallback(vfs::Inode* inode, void* void_arg) {
  OpenContext* arg = (OpenContext*)void_arg;

  uint64_t old_cr3 = Getcr3();
  bool switch_tables = old_cr3 != arg->proc->cr3;

  arg->blocked_queue.UnblockHead();

  if (!inode) {
    if (switch_tables) {
      Setcr3(arg->proc->cr3);
    }
    arg->params->fd_writeback = -1;
    if (switch_tables) {
      Setcr3(old_cr3);
    }
    return;
  }

  DiskFile* new_file = new DiskFile(inode->Open());
  ipc::Pipe* new_pipe = new_file->Open(ipc::RDWR);
  int new_fd = proc::AddPipeToProc(arg->proc, new_pipe);

  if (switch_tables) {
    Setcr3(arg->proc->cr3);
  }
  arg->params->fd_writeback = new_fd;
  if (switch_tables) {
    Setcr3(old_cr3);
  }
}

static void HandleSyscallOpen(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  SyscallOpenParams* params = (SyscallOpenParams*)param_1;
  stdj::string filepath_string(params->filepath);
  vfs::Filepath filepath(filepath_string);

  OpenContext* arg = new OpenContext();
  arg->params = params;
  arg->blocked_queue.BlockCurrentProcNoNesting();
  arg->proc = proc::GetCurrentProc();
  vfs::FindFile(vfs::GetRootDirectory(), filepath, FindFileCallback, arg);
}

void InitFcntl() {
  SetSyscallHandler(SYSCALL_OPEN, HandleSyscallOpen);
}

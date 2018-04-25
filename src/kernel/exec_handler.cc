#include "exec_handler.h"

#include "syscall_handler.h"
#include "page.h"
#include "kmalloc.h"
#include "elf.h"
#include "printk.h"
#include "syscall.h"
#include "asm.h"
#include "proc.h"
#include "user.h"
#include "string.h"
#include "kernel/vfs/find_file.h"
#include "kernel/vfs/file.h"
#include "kernel/vfs/globals.h"
#include "unistd.h"

struct ExecContext {
  proc::BlockedQueue proc_queue;
  // TODO what if proc somehow gets deleted? use weak pointer?
  proc::ProcContext* proc;
  uint8_t* file_data;
  vfs::File* file;
  vfs::Inode* inode;
  SyscallExecParams* params;
};

static void ReadFileCallback(void* void_arg) {
  ExecContext* arg = (ExecContext*)void_arg;

  ELFInfo elf_info = ELFGetInfo(arg->file_data, arg->file->GetSize());
  if (elf_info.success) {
    proc::ExecProc(arg->proc, elf_info, arg->file_data, arg->params->argv);

    bool change_cr3 = arg->proc->cr3 != Getcr3();
    uint64_t old_cr3 = Getcr3();
    if (change_cr3) {
      Setcr3(arg->proc->cr3);
    }
    arg->params->status_writeback = 0;
    if (change_cr3) {
      Setcr3(old_cr3);
    }
  } else {
    printk("HandleSyscallExec failed to exec\n");
  }

  arg->proc_queue.UnblockHead();

  kfree(arg->file_data);
  arg->file->Close();
  kfree(arg->file);
  kfree(arg->inode);
  delete arg;
}

static void FindFileCallback(vfs::Inode* inode, void* void_arg) {
  ExecContext* arg = (ExecContext*)void_arg;
  arg->inode = inode;

  if (arg->inode) {
    arg->file = inode->Open();
    if (arg->file) {
      arg->file_data = (uint8_t*)kmalloc(arg->file->GetSize());
      arg->file->Read(arg->file_data, arg->file->GetSize(), ReadFileCallback,
                      arg);
      return;
    }
  }

  printk("HandleSyscallExec FindFileCallback failed to find file\n");
  bool change_cr3 = arg->proc->cr3 != Getcr3();
  uint64_t old_cr3 = Getcr3();
  if (change_cr3) {
    Setcr3(arg->proc->cr3);
  }
  arg->params->status_writeback = -1;
  if (change_cr3) {
    Setcr3(old_cr3);
  }
  arg->proc_queue.UnblockHead();
  delete arg;
}

// the proc which called this may or may not be in user mode,
// but we will set it to user mode manually
static void HandleSyscallExec(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  SyscallExecParams* params = (SyscallExecParams*)param_1;
  stdj::string input_filepath(params->filepath);
  vfs::Filepath filepath(input_filepath);

  ExecContext* arg = new ExecContext();
  arg->proc = proc::GetCurrentProc();
  arg->proc_queue.BlockCurrentProcNoNesting();
  arg->file_data = 0;
  arg->file = 0;
  arg->inode = 0;
  arg->params = params;
  FindFile(vfs::GetRootDirectory(), filepath, FindFileCallback, arg);
}

void InitExec() {
  SetSyscallHandler(SYSCALL_EXEC, HandleSyscallExec);
}

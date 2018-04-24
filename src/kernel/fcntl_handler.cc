#include "fcntl_handler.h"

#include "syscall.h"
#include "syscall_handler.h"
#include "fcntl.h"
#include "proc.h"
#include "vfs/find_file.h"

struct OpenContext {
  SyscallOpenParams* params;
};

static void FindFileCallback(vfs::Inode* inode, void* void_arg) {
  OpenContext* arg = (OpenContext*)arg;
}

static void HandleSyscallOpen(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  SyscallOpenParams* params = (SyscallOpenParams*)param_1;
  stdj::string filepath_string(params->filepath);
  vfs::Filepath filepath(filepath_string);

  OpenContext* arg = new OpenContext();
  arg->parms = params;
  vfs::FindFile(vfs::GetRootDirectory(), filepath, FindFileCallback, arg);
}

void InitFcntl() {
  SetSyscallHandler(SYSCALL_OPEN, HandleSyscallOpen);
}

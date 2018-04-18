#include "getcwd_handler.h"

#include "syscall_handler.h"
#include "syscall.h"
#include "vfs/filepath.h"
#include "proc.h"
#include "printk.h"

static void HandleGetcwd(uint64_t interrupt_number,
                         uint64_t param_1,
                         uint64_t param_2,
                         uint64_t param_3) {
  SyscallGetcwdParams* params = (SyscallGetcwdParams*)param_1;
  stdj::string filepath = proc::GetCurrentProc()->working_directory_.ToString();
  strncpy(params->buf, filepath.c_str(), params->size);
  params->retval_writeback = params->buf;
}

static void HandleChdir(uint64_t interrupt_number,
                        uint64_t param_1,
                        uint64_t param_2,
                        uint64_t param_3) {
  SyscallChdirParams* params = (SyscallChdirParams*)param_1;
  vfs::Filepath filepath(params->path);
  if (*(params->path) == '/') {
    proc::GetCurrentProc()->working_directory_ = filepath;
  } else {
    proc::GetCurrentProc()->working_directory_.Append(filepath);
  }
  params->status_writeback = 0;
}

void InitGetcwd() {
  SetSyscallHandler(SYSCALL_GETCWD, HandleGetcwd);
  SetSyscallHandler(SYSCALL_CHDIR, HandleChdir);
}

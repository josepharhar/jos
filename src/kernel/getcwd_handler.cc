#include "getcwd_handler.h"

#include "syscall_handler.h"
#include "syscall.h"
#include "vfs/filepath.h"
#include "proc.h"

static void HandleGetcwd(uint64_t interrupt_number,
                         uint64_t param_1,
                         uint64_t param_2,
                         uint64_t param_3) {
  SyscallGetcwdParams* params = (SyscallGetcwdParams*)param_1;
}

static void HandleChdir(uint64_t interrupt_number,
                        uint64_t param_1,
                        uint64_t param_2,
                        uint64_t param_3) {
  SyscallChdirParams* params = (SyscallChdirParams*)param_1;
  vfs::Filepath filepath(params->path);
  printk("changing pid %d working_directory to: %s\n",
         proc::GetCurrentProc()->pid, filepath);
  proc::GetCurrentProc()->working_directory_ = filepath;
  params.status_writeback = 0;
}

void InitGetcwd() {
  SetSyscallHandler(SYSCALL_GETCWD, HandleGetcwd);
  SetSyscallHandler(SYSCALL_CHDIR, HandleChdir);
}

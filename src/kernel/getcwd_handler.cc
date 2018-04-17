#include "getcwd_handler.h"

#include "syscall_handler.h"
#include "syscall.h"

static void HandleGetcwd(uint64_t interrupt_number,
    uint64_t param_1,
    uint64_t param_2,
    uint64_t param_3) {

}

static void HandleChdir(uint64_t interrupt_number,
    uint64_t param_1,
    uint64_t param_2,
    uint64_t param_3) {

}

void InitGetcwd() {
  SetSyscallHandler(SYSCALL_GETCWD, HandleGetcwd);
  SetSyscallHandler(SYSCALL_CHDIR, HandleChdir);
}

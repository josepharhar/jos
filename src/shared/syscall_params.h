#ifndef SHARED_SYSCALL_PARAMS_H_
#define SHARED_SYSCALL_PARAMS_H_

struct SyscallExitParams {
  int exit_status;
};
struct SyscallRdWrParams {
  int fd;
  uint8_t* buffer;
  int size;
  int size_writeback;
};
struct SyscallExecParams {
  char* filepath;
  char** argv;
  int status_writeback;
};
struct SyscallGetcwdParams {
  char* buf;
  int size;
  char* retval_writeback;
};
struct SyscallChdirParams {
  const char* path;
  int status_writeback;
};
struct SyscallWaitParams {
  int* exit_status_writeback;
  int pid_writeback;
};

#endif  // SHARED_SYSCALL_PARAMS_H_

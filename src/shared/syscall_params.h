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

struct SyscallOpendirParams {
  const char* filepath;
  uint64_t id_writeback;
  bool success_writeback;
};
struct SyscallReaddirParams {
  uint64_t id;
  char filename_writeback[256];
  bool success_writeback;
  bool end_of_files_writeback;
};
struct SyscallClosedirParams {
  uint64_t id;
  int status_writeback;
};

struct SyscallOpenParams {
  const char* filepath;
  int flags;
  int fd_writeback;
};

struct SyscallPingParams {
  char* ip_addr;
  int status_writeback;
};

#endif  // SHARED_SYSCALL_PARAMS_H_

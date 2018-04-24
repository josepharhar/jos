#ifndef SHARED_FCNTL_H_
#define SHARED_FCNTL_H_

struct SyscallOpenParams {
  const char* filepath;
  int flags;
  int fd_writeback;
};
int open(const char* pathname, int flags);

#endif  // SHARED_FCNTL_H_

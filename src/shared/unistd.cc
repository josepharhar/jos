#include "unistd.h"

#include "clone.h"
#include "syscall.h"

pid_t fork() {
  return clone(0, 0, CLONE_FILES);
}

pid_t getpid() {
  static uint64_t pid;
  Syscall(SYSCALL_GETPID, (uint64_t)&pid);
  return (int)pid;
}

void exit() {
  Syscall(SYSCALL_EXIT);
}

int write(int fd, const void* write_buffer, int write_size) {
  SyscallRdWrParams params;
  params.fd = fd;
  params.buffer = (uint8_t*)write_buffer;
  params.size = write_size;
  params.size_writeback = -1;

  Syscall(SYSCALL_WRITE, (uint64_t)&params, 0, 0);
  return params.size_writeback;
}

int read(int fd, void* read_buffer, int read_size) {
  SyscallRdWrParams params;
  params.fd = fd;
  params.buffer = (uint8_t*)read_buffer;
  params.size = read_size;
  params.size_writeback = -1;

  Syscall(SYSCALL_READ, (uint64_t)&params, 0, 0);
  return params.size_writeback;
}

int close(int fd) {
  Syscall(SYSCALL_CLOSE, (uint64_t)fd, 0, 0);

  // TODO
  return -1;
}

int pipe(int pipefd[2]) {
  Syscall(SYSCALL_PIPE, (uint64_t)pipefd, 0, 0);

  // TODO
  return -1;
}

void exec(const char* filename) {
  Syscall(SYSCALL_EXEC, (uint64_t) filename);
}

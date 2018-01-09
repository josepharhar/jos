#ifndef SHARED_IPC_H_
#define SHARED_IPC_H_

#include "stdint.h"

struct SyscallRdWrParams {
  int fd;
  uint8_t* buffer;
  int size;
  int size_writeback;
};

int write(int fd, const void* write_buffer, int write_size);
int read(int fd, void* read_buffer, int read_size);

int close(int fd);

int pipe(int pipefd[2]);

#endif  // SHARED_IPC_H_

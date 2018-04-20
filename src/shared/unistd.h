#ifndef JOS_UNISTD_H_
#define JOS_UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef JOS

#include "stdint.h"

typedef int pid_t;

pid_t fork();
pid_t getpid();

void exit();

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

void exec(const char* filepath);
int execv(const char* path, char* const argv[]);

struct SyscallGetcwdParams {
  char* buf;
  int size;
  char* retval_writeback;
};
char* getcwd(char* buf, int size);
struct SyscallChdirParams {
  const char* path;
  int status_writeback;
};
int chdir(const char* path);

#endif  // JOS

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_UNISTD_H_

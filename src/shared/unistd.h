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

struct SyscallExitParams {
  int exit_status;
};
void exit(int status);

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

struct SyscallExecParams {
  char* filepath;
  char** argv;
  int status_writeback;
};
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

struct SyscallWaitParams {
  int* exit_status_writeback;
  int pid_writeback;
};
pid_t wait(int* status);

#endif  // JOS

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_UNISTD_H_

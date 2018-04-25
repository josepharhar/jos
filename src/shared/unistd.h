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

void exit(int status);

int write(int fd, const void* write_buffer, int write_size);
int read(int fd, void* read_buffer, int read_size);

int close(int fd);

int pipe(int pipefd[2]);

int execv(const char* path, char* const argv[]);

char* getcwd(char* buf, int size);
int chdir(const char* path);

pid_t wait(int* status);

#endif  // JOS

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_UNISTD_H_

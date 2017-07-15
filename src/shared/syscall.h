#ifndef SYSCALL_H_
#define SYSCALL_H_

#include "stdint.h"

#define SYSCALL_INT 0x80

#define SYSCALL_YIELD 1
#define SYSCALL_EXIT 2
#define SYSCALL_PROC_RUN 3
#define SYSCALL_GETC 4
#define SYSCALL_PUTC 5
// TODO add SYSCALL_PUTS
#define SYSCALL_EXEC 6
#define SYSCALL_FORK 7
#define SYSCALL_GETPID 8
#define SYSCALL_CLONE 9

#define MAX_NUM_SYSCALLS 256

#ifdef __cplusplus
extern "C" {
#endif

void Syscall(uint64_t syscall_num,
             uint64_t syscall_param_1 = 0,
             uint64_t syscall_param_2 = 0,
             uint64_t syscall_param_3 = 0);

#ifdef __cplusplus
}
#endif

#endif  // SYSCALL_H_

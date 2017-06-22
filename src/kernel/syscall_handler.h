#ifndef SYSCALL_HANDLER_H_
#define SYSCALL_HANDLER_H_

#include "stdint.h"

typedef void (*SyscallHandler)(uint64_t syscall_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3);

void SetSyscallHandler(uint64_t syscall_number, SyscallHandler handler);
void InitSyscall();

#ifdef __cplusplus
extern "C" {
#endif

void HandleSyscall(uint64_t interrupt_number,
                   uint64_t error_code,
                   uint64_t syscall_num,
                   uint64_t syscall_param_1,
                   uint64_t syscall_param_2,
                   uint64_t syscall_param_3);

#ifdef __cplusplus
}
#endif

#endif  // SYSCALL_HANDLER_H_

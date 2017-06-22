#include "syscall.h"

#include "asm.h"

// rdi: interrupt number
// rsi: error code
// rdx: syscall number
// rcx: syscall param 1
// r8:  syscall param 2
// r9:  syscall param 3

void Syscall(uint64_t syscall_num,
             uint64_t syscall_param_1,
             uint64_t syscall_param_2,
             uint64_t syscall_param_3) {
  SET_REGISTER("rdx", syscall_num);
  SET_REGISTER("rcx", syscall_param_1);
  SET_REGISTER("r8", syscall_param_2);
  SET_REGISTER("r9", syscall_param_3);
  asm volatile("int $0x80");
}

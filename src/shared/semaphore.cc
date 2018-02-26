#include "semaphore.h"

#include "syscall.h"

void sem_wait(sem_t* semaphore) {
  SyscallSemaphoreRequest request;
  request.type = SEM_WAIT;
  request.semaphore = semaphore;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

void sem_post(sem_t* semaphore) {
  SyscallSemaphoreRequest request;
  request.type = SEM_POST;
  request.semaphore = semaphore;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

void sem_init(sem_t* semaphore) {
  SyscallSemaphoreRequest request;
  request.type = SEM_INIT;
  request.semaphore = semaphore;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

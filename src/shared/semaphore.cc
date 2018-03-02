#include "semaphore.h"

#include "syscall.h"

void sem_wait(sem_t* semaphore) {
  SyscallSemaphoreRequest request;
  request.type = SEM_WAIT;
  request.semaphore = semaphore;
  request.status_writeback = -2;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

void sem_post(sem_t* semaphore) {
  SyscallSemaphoreRequest request;
  request.type = SEM_POST;
  request.semaphore = semaphore;
  request.status_writeback = -2;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

void sem_init(sem_t* semaphore, int pshared, unsigned int value) {
  SyscallSemaphoreRequest request;
  request.type = SEM_OPEN;
  request.semaphore = semaphore;
  request.status_writeback = -2;
  Syscall(SYSCALL_SEMAPHORE, (uint64_t)&request);
}

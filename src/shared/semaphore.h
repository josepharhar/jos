#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

typedef int sem_t;

void sem_wait(sem_t* semaphore);
void sem_post(sem_t* semaphore);
void sem_init(sem_t* semaphore);

enum SemaphoreRequestType {
  SEM_WAIT = 1,
  SEM_POST = 2,
  SEM_INIT = 3,
};
struct SyscallSemaphoreRequest {
  SemaphoreRequestType type;
  sem_t* semaphore;
};

#endif  // SEMAPHORE_H_

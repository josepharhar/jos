#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "jstring.h"

/*typedef struct sem_t {
  char name[256];
} sem_t;*/
typedef int sem_t;

void sem_wait(sem_t* semaphore);
void sem_post(sem_t* semaphore);
void sem_init(sem_t* semaphore, int pshared, unsigned int value);
// void sem_open(const char* name, int oflag, mode_t mode, unsigned int value);
// sem_t* sem_open(const char* name);

enum SemaphoreRequestType {
  SEM_WAIT = 1,
  SEM_POST = 2,
  SEM_INIT = 3,
};
struct SyscallSemaphoreRequest {
  SemaphoreRequestType type;
  sem_t* semaphore;
  int status_writeback;
};

#endif  // SEMAPHORE_H_

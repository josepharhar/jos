#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "jstring.h"

#define SEMAPHORE_MAX_NAME_LENGTH 256

typedef struct sem_t {
  char name[SEMAPHORE_MAX_NAME_LENGTH];
} sem_t;
//typedef int sem_t;

int sem_wait(sem_t* semaphore);
int sem_post(sem_t* semaphore);

void sem_init(sem_t* semaphore, int pshared, unsigned int value);
void sem_destroy(sem_t* semaphore);

// TODO make this return a sem_t* when userspace allocator works
void sem_open(sem_t* semaphore, const char* name);
void sem_close(sem_t* semaphore);

enum SemaphoreRequestType {
  SEM_WAIT = 1,
  SEM_POST = 2,
  SEM_INIT = 3,
  SEM_OPEN = 4,
};
struct SyscallSemaphoreRequest {
  SemaphoreRequestType type;
  sem_t* semaphore;
  int status_writeback;
  const char* name;
};

#endif  // SEMAPHORE_H_

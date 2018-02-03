#include <stdio.h>

#include <sched.h>   // linux clone()
#include <stdlib.h>  // exit()
#include <unistd.h>  // getpid()
#include <semaphore.h>
#include <fcntl.h> // O_CREAT
#include <errno.h>
#include <string.h> // strerror()?

void Parent();

int main(int argc, char** argv) {
  Parent();
}

int test = 0;
char stack[8192];
sem_t semaphore_1;
sem_t semaphore_2;

int NewProc(void* arg) {
  printf("NewProc() pid: %d, arg: %p\n", getpid(), arg);
  printf("NewProc() test: %d\n", test);
  exit(0);
  return 0;
}

int NewThread(void* arg) {
  printf("NewThread() pid: %d, arg: %p\n", getpid(), arg);
  printf("NewThread() test: %d\n", test);

  printf("NewThread() waiting on semaphore_1\n");
  sem_wait(&semaphore_1);
  printf("NewThread() done waiting on semaphore_1\n");

  // unlock semaphore_2
  sem_post(&semaphore_2);
  printf("NewThread() unlocked semaphore_2\n");

  exit(0);
  return 0;
}

void Parent() {
  test = 1;

  printf("main() pid: %d calling clone()\n", getpid());

  if (sem_init(&semaphore_1, 0 /* pshared */, 0 /* init value - locked */)) {
    printf("sem_init() failed, strerror(): %s\n", strerror(errno));
    return;
  }
  if (sem_init(&semaphore_2, 0 /* pshared */, 0 /* init value - locked */)) {
    printf("sem_init() failed, strerror(): %s\n", strerror(errno));
    return;
  }

  // unlock the semaphore
  sem_post(&semaphore_1);

  int arg = 4880;
  int clone_flags = 0;
  // clone_flags |= CLONE_FILES; // copy fd table
  clone_flags |= CLONE_VM; // don't copy page table
  clone(&NewThread, stack + 4096, clone_flags, &arg);

  test = 2;
  //printf("main() set test to 2\n");

  printf("main() waiting for user input\n");
  int input = fgetc(stdin);
  printf("main() read user input: %c\n", (char)input);

  printf("main() waiting on semaphore_2\n");
  sem_wait(&semaphore_2);
}

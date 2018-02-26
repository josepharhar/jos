//#include "jos.h"

#include <stdio.h>
#include <semaphore.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

// TODO make elf loader figure out where address of actual main() is
void mainmain();
int main(int argc, char** argv) {
  mainmain();
  return 0;
}

static const char* semaphore_name = "/jarhar";
static const mode_t semaphore_mode = S_IRWXU | S_IRWXG | S_IRWXO;
// for some reason this value has no effect on the semaphores initial value.
static const unsigned semaphore_init_value = 0;  // 0 = locked, 1 = available.

static sem_t* semaphore = 0;
static int pid = 0;

static void OpenSemaphore() {
  pid = getpid();
  printf("[%d] begin\n", pid);
  semaphore =
      sem_open(semaphore_name, O_CREAT, semaphore_mode, semaphore_init_value);
  printf("[%d] sem_open() returned %p\n", pid, semaphore);
  if (!semaphore) {
    printf("[%d] errno: %d, strerror(): %s\n", pid, errno, strerror(errno));
    exit(1);
  }
}

static void AcquireAndPrint() {
  printf("[%d] calling sem_wait()...\n", pid);
  sem_wait(semaphore);
  printf("[%d] sem_wait() returned\n", pid);

  for (int i = 0; i < 10; i++) {
    sleep(1);
    printf("[%d]", pid);
    fflush(stdout);
  }
  printf("\n");

  printf("[%d] calling sem_post()\n", pid);
  sem_post(semaphore);
}

void mainmain() {
  if (fork()) {
    OpenSemaphore();

    sem_post(semaphore);

    AcquireAndPrint();

    int status;
    wait(&status);

  } else {
    OpenSemaphore();
    AcquireAndPrint();
  }
}

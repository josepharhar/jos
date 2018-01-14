#include <semaphore.h>
//#include <sys/stat.h>
//#include <fcntl.h>

static sem_t semaphore;

static void Parent() {

}

static void Child() {

}

static void ChildChild() {

}

int main(int argc, char** argv) {
  // (shared) memory semaphore: sem_init()
  // named semaphore: sem_open()
  sem_init(&semaphore, 1 /* pshared */, 0 /* value */);

  int pid = fork();
  if (pid) {
    Parent();
  } else {
    int pid2 = fork();
    if (pid2) {
      Child();
    } else {
      ChildChild();
    }
  }
}

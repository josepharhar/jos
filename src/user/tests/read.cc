#include <stdio.h>
#include <unistd.h>

static int fds[2];

static void Parent() {
  int bytes_written = write(fds[1], "asdf", 4);
  printf("Parent() wrote %d bytes\n", bytes_written);

  int status;
  wait(&status);
}

static void Child() {
  char buf[100] = {0};
  int bytes_read = read(fds[0], buf, 100);
  printf("Child() read %d bytes: \"%s\"\n", bytes_read, buf);

  int status;
  wait(&status);
}

static void ChildChild() {
  char buf[100] = {0};
  int bytes_read = read(fds[0], buf, 100);
  printf("ChildChild() read %d bytes: \"%s\"\n", bytes_read, buf);
}

int main(int argc, char** argv) {
  if (pipe(fds)) {
    printf("pipe() returned nonzero\n");
    return 1;
  }

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

  return 0;
}

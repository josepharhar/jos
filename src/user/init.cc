#include "init.h"

#include "io.h"
#include "asm.h"
#include "printu.h"
#include "fork.h"
#include "getpid.h"

int main() {
  Puts("Hello from init\n");

  /*uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printu("rsp: %p\n", rsp);*/

  /*printu("calling interrupt 0x81\n");
  asm volatile ("int $0x81");
  printu("flags: %p\n", get_flags());*/

  printu("calling fork\n");
  uint64_t pid = Fork();
  if (pid) {
    printu("parent proc pid %p with child pid %p\n", Getpid(), pid);
  } else {
    printu("child proc pid %d\n", Getpid());
  }

  //while (1) {}

  while (1) {
    Putc(Getc());
  }
}

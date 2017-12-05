#include "init.h"

#include "io.h"
#include "asm.h"
#include "printu.h"
#include "clone.h"
#include "getpid.h"

static void NewProc();
extern char new_stack[];

//static int asdf = 130;
extern int asdf;

// TODO the first piece of code in this file is what gets run by
// exec(), make it always look for main() somehow instead
int main() {
  Puts("Hello from init\n");
  printu("main() pid: %d\n", getpid());

  /*uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printu("rsp: %p\n", rsp);*/

  /*printu("calling interrupt 0x81\n");
  asm volatile ("int $0x81");
  printu("flags: %p\n", get_flags());*/

  printu("calling clone\n");
  asdf = 248;
  CloneOptions options;
  options.copy_page_table = 1;
  // TODO make start_at_callback = 0 work
  options.start_at_callback = 1;
  //clone(&options, NewProc, new_stack + 2048);
  clone(&options, NewProc, 0);
  printu("main() done calling clone\n");

  printu("%p: %d\n", &asdf, asdf);

  while (1) {
    Putc(Getc());
  }
}

int asdf = 130;
char new_stack[4096];
static void NewProc() {
  Puts("Hello from NewProc()\n");
  printu("NewProc pid: %d\n", getpid());
  asdf = 4880;
  printu("%p: %d\n", &asdf, asdf);

  while (1) {
    Putc(Getc());
  }

  // TODO ending process causes page fault
}

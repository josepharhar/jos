#include "init.h"

#include "io.h"
#include "asm.h"
#include "printu.h"
#include "clone.h"
#include "getpid.h"
#include "shared/ipc.h"
#include "string.h"

// TODO the first piece of code in this file is what gets run by
// exec(), make it always look for main() somehow instead
void proc_testing();
void class_testing();
void ipc_testing();
int main() {
  Puts("Hello from USERSPACE init\n");
  // proc_testing();
  // class_testing();
  ipc_testing();

  Puts("\ninit process ending\n");
  // TODO ProcExit();
}

class Class {
 public:
  Class() { printu("Class::Class()\n"); }
  ~Class() { printu("Class::~Class()\n"); }
  Class(const Class& other) { printu("Class::Class(const Class&)\n"); }
  Class& operator=(const Class& other) {
    printu("Class::operator=(const Class&)\n");
    return *this;
  }

  virtual void PrintClassName() { printu("PrintClassName Class\n"); }
};

class SubClass : public Class {
 public:
  void PrintClassName() override { printu("PrintClassName SubClass\n"); }
};

void class_testing() {
  printu("\n");

  SubClass asdf;
  asdf.PrintClassName();
}

static int asdf = 130;
static char new_stack[4096];

static void NewProc() {
  Puts("Hello from NewProc()\n");
  printu("NewProc pid: %d\n", getpid());
  close(1234);

  printu("NewProc() asdf: %d\n", asdf);

  while (1) {
    Putc(Getc());
  }
}

void proc_testing() {
  asdf = 1;

  printu("calling clone\n");
  /*CloneOptions options;
  options.copy_page_table = 1;
  // TODO make start_at_callback = 0 work
  options.start_at_callback = 1;
  // clone(&options, NewProc, new_stack + 2048);
  clone(&options, NewProc, 0);*/
  clone(&NewProc, new_stack + 2048, CLONE_FILES);

  asdf = 2;
  printu("proc_testing() set asdf = 2\n");

  printu("main() done calling clone\n");
  close(1234);

  while (1) {
    Putc(Getc());
  }
}

static int fds[2];

static void ipcnewproc() {
  printu("Hello from ipcnewproc(), pid: %d\n", getpid());
  printu("ipcnewproc() writing asdf...\n");
  write(fds[1], "asdf", 4);
  printu("ipcnewproc() wrote asdf\n");

  while (1) {
    Putc(Getc());
  }
}

void ipc_testing() {
  printu("ipc_testing()\n");

  pipe(fds);

  printu("calling clone()...\n");
  clone(ipcnewproc, new_stack + 2048, CLONE_FILES);
  printu("ipc_testing returned from clone(), pid: %d\n", getpid());

  printu("ipc_testing() going to block on reading...\n");
  char buffer[100];
  memset(buffer, 0, 100);
  int bytes_read = read(fds[0], buffer, 4);
  printu("ipc_testing() read() returned, bytes_read: %d, buffer: %s\n",
         bytes_read, buffer);

  while (1) {
    Putc(Getc());
  }
}

#include "init.h"

#include "getc.h"
#include "asm.h"
#include "stdio.h"
#include "clone.h"
#include "shared/ipc.h"
#include "string.h"
#include "unistd.h"

// TODO the first piece of code in this file is what gets run by
// exec(), make it always look for main() somehow instead
void proc_testing();
void class_testing();
void ipc_testing();
void stack_testing();
void fork_testing();
void preempt_testing();
void semaphore_testing();
int main() {
  Puts("Hello from USERSPACE init\n");
  // while (1);
  // proc_testing();
  // class_testing();
  ipc_testing();
  // stack_testing();
  // fork_testing();
  // preempt_testing();
  // semaphore_testing();

  while (1) {
    Putc(Getc());
  }
  Puts("\ninit process ending\n");
  // TODO ProcExit();
}

class Class {
 public:
  Class() { printj("Class::Class()\n"); }
  ~Class() { printj("Class::~Class()\n"); }
  Class(const Class& other) { printj("Class::Class(const Class&)\n"); }
  Class& operator=(const Class& other) {
    printj("Class::operator=(const Class&)\n");
    return *this;
  }

  virtual void PrintClassName() { printj("PrintClassName Class\n"); }
};

class SubClass : public Class {
 public:
  void PrintClassName() override { printj("PrintClassName SubClass\n"); }
};

void class_testing() {
  printj("\n");

  SubClass asdf;
  asdf.PrintClassName();
}

static int asdf = 130;
static char new_stack[4096];

static void NewProc() {
  Puts("Hello from NewProc()\n");
  printj("NewProc pid: %d\n", getpid());
  close(1234);

  printj("NewProc() asdf: %d\n", asdf);

  while (1) {
    Putc(Getc());
  }
}

void proc_testing() {
  asdf = 1;

  printj("calling clone\n");
  /*CloneOptions options;
  options.copy_page_table = 1;
  // TODO make start_at_callback = 0 work
  options.start_at_callback = 1;
  // clone(&options, NewProc, new_stack + 2048);
  clone(&options, NewProc, 0);*/
  clone(&NewProc, new_stack + 2048, CLONE_FILES);

  asdf = 2;
  printj("proc_testing() set asdf = 2\n");

  printj("main() done calling clone\n");
  close(1234);

  while (1) {
    Putc(Getc());
  }
}

static int fds[2];

static void ipcnewproc() {
  printj("Hello from ipcnewproc(), pid: %d\n", getpid());
  printj("ipcnewproc() writing asdf...\n");
  write(fds[1], "asdf", 4);
  printj("ipcnewproc() wrote asdf\n");

  while (1) {
    Putc(Getc());
  }
}

void ipc_testing() {
  printj("ipc_testing()\n");

  pipe(fds);

  printj("calling clone()...\n");
  clone(ipcnewproc, new_stack + 2048, CLONE_FILES);
  printj("ipc_testing returned from clone(), pid: %d\n", getpid());

  printj("ipc_testing() going to block on reading...\n");
  char buffer[100];
  memset(buffer, 0, 100);
  int bytes_read = read(fds[0], buffer, 4);
  printj("ipc_testing() read() returned, bytes_read: %d, buffer: %s\n",
         bytes_read, buffer);

  while (1) {
    Putc(Getc());
  }
}

static char new_stack_1[8192];
static void stackforkproc() {
  uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printj("stackforkproc() begin. rsp: %p, pid: %d\n", rsp, getpid());

  int pid = getpid();
  while (1) {
    // printj("pid: %d\n", pid);
  }
}
static void stackcloneproc() {
  uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printj("stackcloneproc() begin. rsp: %p, pid: %d\n", rsp, getpid());

  printj("stackcloneproc() calling clone()...\n");
  clone(stackforkproc, 0, CLONE_FILES);
  printj("stackcloneproc() done calling clone()\n");

  int pid = getpid();
  while (1) {
    // printj("pid: %d\n", pid);
  }
}
void stack_testing() {
  printj("stack_testing() begin. calling clone()...\n");
  clone(stackcloneproc, new_stack_1 + 1024, CLONE_FILES);
  printj("stack_testing() returned from clone()\n");

  while (1) {
    Putc(Getc());
  }
}

void fork_testing() {
  printj("fork_testing() begin. calling fork()...\n");
  int fork_retval = fork();
  // int pid = clone(asdfasdfasdf, 0, 0);
  // int pid = 1234;

  static int pid = getpid();
  printj("fork(): %d, pid: %d\n", fork_retval, pid);

  while (1) {
    char input = Getc();
    printj("pid %d input '%c'\n", pid, input);
  }
}

void preempt_testing() {
  int counter = 0;

  if (fork()) {
    counter++;
    if (fork()) {
      counter++;
      if (fork()) {
        counter++;
      }
    }
  }

  const char* indent = "";
  if (counter == 1) {
    indent = " ";
  } else if (counter == 2) {
    indent = "  ";
  } else if (counter == 3) {
    indent = "   ";
  }

  int pid = getpid();
  while (1) {
    printj("%spid: %d\n", indent, pid);
  }
}

void semaphore_testing() {}

#include "init.h"

#include "io.h"
#include "asm.h"
#include "printu.h"
#include "clone.h"
#include "getpid.h"
#include "shared/ipc.h"

// TODO the first piece of code in this file is what gets run by
// exec(), make it always look for main() somehow instead
void proc_testing();
void class_testing();
int main() {
  Puts("Hello from USERSPACE init\n");
  proc_testing();
  //class_testing();
  //ipc_testing();

  Puts("\ninit process ending\n");
  // TODO ProcExit();
}


class Class {
 public:
  Class() {
    printu("Class::Class()\n");
  }
  ~Class() {
    printu("Class::~Class()\n");
  }
  Class(const Class& other) {
    printu("Class::Class(const Class&)\n");
  }
  Class& operator=(const Class& other) {
    printu("Class::operator=(const Class&)\n");
    return *this;
  }

  virtual void PrintClassName() {
    printu("PrintClassName Class\n");
  }
};

class SubClass : public Class {
 public:
  void PrintClassName() override {
    printu("PrintClassName SubClass\n");
  }
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
  CloneOptions options;
  options.copy_page_table = 1;
  // TODO make start_at_callback = 0 work
  options.start_at_callback = 1;
  //clone(&options, NewProc, new_stack + 2048);
  clone(&options, NewProc, 0);

  asdf = 2;
  printu("proc_testing() set asdf = 2\n");

  printu("main() done calling clone\n");
  close(1234);

  while (1) {
    Putc(Getc());
  }
}


static void ipcnewproc() {
  printu("Hello from ipcnewproc(), pid: %d\n", getpid());
}

void ipc_testing() {
  printu("ipc_testing()\n");

  char buffer1[100];
  char buffer2[100];

  int fds[2];
  pipe(fds);
  // TODO
  //write(fds[1], "asdf", 

  CloneOptions clone_options;
  clone_options.copy_page_table = 1;
  clone_options.start_at_callback = 1;
  printu("calling clone()...\n");
  clone(&clone_options, ipcnewproc, 0);
  printu("ipc_testing returned from clone(), pid: %d\n", getpid());
}

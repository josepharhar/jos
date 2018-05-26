#include "init.h"

#include "getc.h"
#include "asm.h"
#include "stdio.h"
#include "clone.h"
#include "string.h"
#include "unistd.h"
#include "semaphore.h"

// TODO the first piece of code in this file is what gets run by
// exec(), make it always look for main() somehow instead
void proc_testing();
void class_testing();
void ipc_testing();
void stack_testing();
void fork_testing();
void preempt_testing();
void semaphore_testing();
void exit_testing();
void exec_testing();
int main() {
  Puts("Hello from USERSPACE init\n");
  // while (1);
  // proc_testing();
  // class_testing();
  // ipc_testing();
  // stack_testing();
  // fork_testing();
  // preempt_testing();
  // semaphore_testing();
  // exit_testing();
  // exec_testing();
  
  /*char buf[30];
  memset(buf, 0, 30);
  getcwd(buf, 30);
  printf("getcwd(): \"%s\"\n", buf);
  printf("chdir(\"user\"): %d\n", chdir("user"));
  memset(buf, 0, 30);
  getcwd(buf, 30);
  printf("getcwd(): \"%s\"\n", buf);*/

  printf("init calling exec(\"/user/jshell\")\n");
  execv("/user/jshell", 0);
  printf("init exec failed!\n");
  exit(1);

  while (1) {
    Putc(Getc());
  }
  Puts("\ninit process ending\n");
  // TODO ProcExit();
}

class Class {
 public:
  Class() { printf("Class::Class()\n"); }
  ~Class() { printf("Class::~Class()\n"); }
  Class(const Class& other) { printf("Class::Class(const Class&)\n"); }
  Class& operator=(const Class& other) {
    printf("Class::operator=(const Class&)\n");
    return *this;
  }

  virtual void PrintClassName() { printf("PrintClassName Class\n"); }
};

class SubClass : public Class {
 public:
  void PrintClassName() override { printf("PrintClassName SubClass\n"); }
};

void class_testing() {
  printf("\n");

  SubClass asdf;
  asdf.PrintClassName();
}

static int asdf = 130;
static char new_stack[4096];

static void NewProc() {
  Puts("Hello from NewProc()\n");
  printf("NewProc pid: %d\n", getpid());
  close(1234);

  printf("NewProc() asdf: %d\n", asdf);

  while (1) {
    Putc(Getc());
  }
}

void proc_testing() {
  asdf = 1;

  printf("calling clone\n");
  /*CloneOptions options;
  options.copy_page_table = 1;
  // TODO make start_at_callback = 0 work
  options.start_at_callback = 1;
  // clone(&options, NewProc, new_stack + 2048);
  clone(&options, NewProc, 0);*/
  clone(&NewProc, new_stack + 2048, CLONE_FILES);

  asdf = 2;
  printf("proc_testing() set asdf = 2\n");

  printf("main() done calling clone\n");
  close(1234);

  while (1) {
    Putc(Getc());
  }
}

static int fds[2];

static void ipcnewproc() {
  printf("Hello from ipcnewproc(), pid: %d\n", getpid());
  printf("ipcnewproc() writing asdf...\n");
  write(fds[1], "asdf", 4);
  printf("ipcnewproc() wrote asdf\n");

  while (1) {
    Putc(Getc());
  }
}

void ipc_testing_old() {
  printf("ipc_testing()\n");

  pipe(fds);

  printf("calling clone()...\n");
  clone(ipcnewproc, new_stack + 2048, CLONE_FILES);
  printf("ipc_testing returned from clone(), pid: %d\n", getpid());

  printf("ipc_testing() going to block on reading...\n");
  char buffer[100];
  memset(buffer, 0, 100);
  int bytes_read = read(fds[0], buffer, 4);
  printf("ipc_testing() read() returned, bytes_read: %d, buffer: %s\n",
         bytes_read, buffer);

  while (1) {
    Putc(Getc());
  }
}

void ipc_testing() {
  printf("ipc_testing()\n");
  pipe(fds);

  if (fork()) {
    write(fds[1], "hello", 5);
    while (1) {
      char input = Getc();
      write(fds[1], &input, 1);
    }
  } else {
    while (1) {
      char buffer[100];
      memset(buffer, 0, 100);
      int bytes_read = read(fds[0], buffer, 99);
      printf("read from pipe: \"%s\"\n", buffer);
    }
  }
}

static char new_stack_1[8192];
static void stackforkproc() {
  uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printf("stackforkproc() begin. rsp: %p, pid: %d\n", rsp, getpid());

  int pid = getpid();
  while (1) {
    // printf("pid: %d\n", pid);
  }
}
static void stackcloneproc() {
  uint64_t rsp;
  GET_REGISTER("rsp", rsp);
  printf("stackcloneproc() begin. rsp: %p, pid: %d\n", rsp, getpid());

  printf("stackcloneproc() calling clone()...\n");
  clone(stackforkproc, 0, CLONE_FILES);
  printf("stackcloneproc() done calling clone()\n");

  int pid = getpid();
  while (1) {
    // printf("pid: %d\n", pid);
  }
}
void stack_testing() {
  printf("stack_testing() begin. calling clone()...\n");
  clone(stackcloneproc, new_stack_1 + 1024, CLONE_FILES);
  printf("stack_testing() returned from clone()\n");

  while (1) {
    Putc(Getc());
  }
}

void fork_testing() {
  printf("fork_testing() begin. calling fork()...\n");
  int fork_retval = fork();
  // int pid = clone(asdfasdfasdf, 0, 0);
  // int pid = 1234;

  static int pid = getpid();
  printf("fork(): %d, pid: %d\n", fork_retval, pid);

  while (1) {
    char input = Getc();
    printf("pid %d input '%c'\n", pid, input);
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
    for (int i = 0; i < 10000000; i++)
      ;
    printf("%spid: %d\n", indent, pid);
  }
}

sem_t semaphore;
int pid;
static void semaphore_waiting() {
  printf("[%d] calling sem_wait()...\n", pid);
  int wait_retval = sem_wait(&semaphore);
  printf("[%d] sem_wait() returned %d\n", pid, wait_retval);

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 100000000; j++)
      ;
    printf("[%d]", pid);
  }
  printf("\n");

  printf("[%d] calling sem_post()\n", pid);
  int post_retval = sem_post(&semaphore);
  printf("[%d] sem_post() returned %d\n", pid, post_retval);
}
void semaphore_testing() {
  if (fork()) {
    pid = getpid();
    printf("[%d] calling sem_open()\n", pid);
    sem_open(&semaphore, "/jarhar");
    printf("[%d] sem_open() returned\n", pid);

    int post_retval = sem_post(&semaphore);
    printf("[%d] sem_post() returned %d\n", pid, post_retval);

    semaphore_waiting();

  } else {
    pid = getpid();
    sem_open(&semaphore, "/jarhar");
    printf("[%d] sem_open() returned\n", pid);

    semaphore_waiting();
  }

  while (1) {
    Putc(Getc());
  }
}

void exit_testing() {
  printf("exit_testing()\n");

  if (!fork()) {
    printf("pid %d calling exit()...\n", getpid());
    exit(0);
    printf("RETURNED FROM exit()!!!\n");
  }

  if (fork()) {
    printf("pid %d calling exit()...\n", getpid());
    exit(0);
    printf("RETURNED FROM exit()!!!\n");
  }
  if (fork()) {
    printf("pid %d calling exit()...\n", getpid());
    exit(0);
    printf("RETURNED FROM exit()!!!\n");
  }

  while (1) {
    Putc(Getc());
  }
}

void exec_testing() {
  printf("exec_testing()\n");

  if (!fork()) {
    printf("pid %d calling exec(\"/user/test/hello\")...\n", getpid());
    execv("/user/test/hello", 0);
  }

  while (1) {
    Putc(Getc());
  }
}

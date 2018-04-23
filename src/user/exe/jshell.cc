#include "jos.h"

#include "stdio.h"
#include "unistd.h"
#include "jstring.h"
#include "getc.h"
#include "dirent.h"
#include "stdlib.h"

int main() {
  printf("welcome to jshell\n");

  while (1) {
    char buf[50];
    memset(buf, 50, 0);
    getcwd(buf, 50);
    printf("%s > ", buf);

    stdj::string input_string;

    char input = 0;
    while (input != '\n') {
      input = Getc();
      printf("%c", input);
      if (input != '\n') {
        input_string.Add(input);
      }
    }

    stdj::Array<stdj::string> args = input_string.Split(" ");
    if (args.Size() && args.Get(0) == stdj::string("cd")) {
      if (args.Size() != 2) {
        printf("usage: cd <directory>\n");
      } else {
        stdj::string dir = args.Get(1);
        if (chdir(dir.c_str())) {
          printf("failed to chdir() to \"%s\"\n", dir.c_str());
        }
      }

    } else if (args.Size() && args.Get(0) == stdj::string("exit")) {
      exit(0);

    } else if (args.Size()) {
      if (!fork()) {
        char** argv = (char**)malloc(sizeof(uint64_t) * (args.Size() + 1));
        for (int i = 0; i < args.Size(); i++) {
          stdj::string arg = args.Get(i);
          char* arg_string = (char*)malloc(arg.Size() + 1);
          strcpy(arg_string, arg.c_str());
          argv[i] = arg_string;
        }
        argv[args.Size() + 1] = 0;
        stdj::string prog_filepath = args.Get(0);
        execv(prog_filepath.c_str(), argv);
        
        printf("jshell: exec(\"%s\") failed!\n", prog_filepath.c_str());
        exit(1);

      } else {
        printf("jshell: calling wait()\n");
        int status;
        wait(&status);
        printf("jshell: finished waiting, status: %d\n", status);
      }
    }
  }

  while (1) {
    Putc(Getc());
  }
  return 0;
}

#include "jos.h"

#include "stdio.h"
#include "unistd.h"
#include "jstring.h"
#include "getc.h"
#include "dirent.h"

void ls();

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

    } else if (args.Size() && args.Get(0) == stdj::string("ls")) {
      char argv[2][10];
      strcpy(argv[0], "/user/ls");
      strcpy(argv[1], "/user");
      execv("/user/ls", (char**)argv);
      //ls();
    }
  }

  while (1) {
    Putc(Getc());
  }
  return 0;
}

void ls() {
  DIR* dir = opendir("/user");
  if (!dir) {
    printf("opendir(/user) failed\n");
    return;
  }

  dirent* ent = readdir(dir);
  while (ent) {
    printf("ent->d_name: \"%s\"\n", ent->d_name);
    ent = readdir(dir);
  }
  closedir(dir);
}

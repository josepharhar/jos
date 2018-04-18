#include "jos.h"

#include "stdio.h"
#include "unistd.h"
#include "jstring.h"
#include "getc.h"

int main() {
  printf("welcome to jshell\n");

  char buf[50];
  memset(buf, 50, 0);
  getcwd(buf, 50);
  printf("%s > ", buf);

  stdj::string input_string;

  char input = 0;
  while (input != '\n') {
    input = Getc();
    input_string.Add(input);
    printf("%c", input);
  }

  printf("jshell got input string: \"%s\"\n", input_string.c_str());

  while (1) {
    Putc(Getc());
  }
  return 0;
}

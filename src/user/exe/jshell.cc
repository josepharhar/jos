#include "jos.h"

#include "stdio.h"
#include "unistd.h"
#include "jstring.h"
#include "getc.h"

int main() {
  printf("jshell > ");

  stdj::string input_string;

  char input = 0;
  while (input != '\n') {
    printf("1\n");
    input = Getc();
    printf("2\n");
    input_string.Add(input);
    printf("%c", input);
  }

  printf("jshell got input string: \"%s\"\n", input_string.c_str());

  while (1) {
    Putc(Getc());
  }
  return 0;
}

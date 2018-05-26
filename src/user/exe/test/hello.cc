#include "stdio.h"
#include "unistd.h"

int main() {
  printf("hello from hello.cc\n");
  printf("hello.cc calling exit()...\n");
  exit(0);
  return 0;
}

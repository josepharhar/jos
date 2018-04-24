#include "jos.h"

#include "fcntl.h"
#include "stdio.h"
#include "unistd.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage: cat <file>\n");
    exit(1);
  }

  char* filename = argv[1];
  int fd = open(filename, 0);
  if (fd < 0) {
    printf("cat: failed to open \"%s\"\n", filename);
    exit(1);
  }

  char file_input = 0;
  while (read(fd, &file_input, 1)) {
    printf("%c", file_input);
  }
  exit(0);
}

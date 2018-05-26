#include "stdio.h"
#include "unistd.h"
#include "dirent.h"
#include "stdlib.h"

int main(int argc, char** argv) {
  char* filepath_to_print = 0;
  if (argc > 1) {
    filepath_to_print = argv[1];
  } else {
    filepath_to_print = (char*)malloc(256);
    getcwd(filepath_to_print, 256);
  }

  printf("ls: printing contents of \"%s\"\n", filepath_to_print);

  DIR* dir = opendir(filepath_to_print);
  if (!dir) {
    printf("ls: opendir() failed\n");
  }

  dirent* ent = readdir(dir);
  while (ent) {
    printf("%s\n", ent->d_name);
    ent = readdir(dir);
  }
  closedir(dir);

  exit(0);
}

#include "stdio.h"
#include "unistd.h"
#include "ping.h"
#include "stdlib.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ping <address>\n");
    return 1;
  }

  char* ip_address = argv[1];
  int ping_retval = ping(ip_address);
  printf("ping returned %d\n", ping_retval);
  exit(0);
  return 0;
}

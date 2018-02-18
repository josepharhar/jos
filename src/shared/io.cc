#include "io.h"

#include "syscall.h"

char Getc() {
 static char input;
 Syscall(SYSCALL_GETC, (uint64_t) &input);
 return input;
}

void Putc(char output) {
  Syscall(SYSCALL_PUTC, output);
}

void Puts(const char* string) {
  while (*string) {
    Putc(*string);
    string++;
  }
}

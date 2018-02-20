#include "getc.h"

#include "syscall.h"

char Getc() {
  SyscallGetcParams params;
  Syscall(SYSCALL_GETC, (uint64_t)&params);
  return params.character_writeback;
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

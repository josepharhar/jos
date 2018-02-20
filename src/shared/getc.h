#ifndef GETC_H_
#define GETC_H_

struct SyscallGetcParams {
  char character_writeback;
};

char Getc();
void Putc(char output);
void Puts(const char* string);

#endif  // GETC_H_

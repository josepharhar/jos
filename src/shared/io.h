#ifndef IO_H_
#define IO_H_

struct SyscallGetcParams {
  char character_writeback;
};

char Getc();
void Putc(char output);
void Puts(const char* string);

#endif  // IO_H_

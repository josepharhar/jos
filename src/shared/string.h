#ifndef STRING_H_
#define STRING_H_

#include "stdint.h"

#ifdef TEST
#include <string.h>
#else
void* memset(void* destination, unsigned char value, int num_bytes);
void* memcpy(void* destination, const void* src, int num_bytes);
void* memmove(void* destination, const void* src, int num_bytes);
int memcmp(const void* one, const void* two, uint64_t num_bytes);
int strcmp(const void* one, const void* two);
char* strcpy(char* dest, const char* src);
char* strncpy(char* destination, const char* src, int num_bytes);
int strlen(const char* s);
#endif  // TEST

#endif  // STRING_H_

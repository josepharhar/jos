#include "string.h"

#include "stdint.h"
#include "stdlib.h"

void* memset(void* destination, unsigned char value, int num_bytes) {
  unsigned char* char_destination = (unsigned char*)destination;

  for (int i = 0; i < num_bytes; i++) {
    char_destination[i] = value;
  }

  return destination;
}

void* memcpy(void* destination, const void* src, int num_bytes) {
  uint8_t* char_dest = (uint8_t*)destination;
  uint8_t* char_src = (uint8_t*)src;

  for (int i = 0; i < num_bytes; i++) {
    char_dest[i] = char_src[i];
  }

  return destination;
}

void* memmove(void* destination, const void* src, int num_bytes) {
  uint8_t* char_dest = (uint8_t*)destination;
  uint8_t* char_src = (uint8_t*)src;
  uint8_t* temp = (uint8_t*)malloc(num_bytes);

  for (int i = 0; i < num_bytes; i++) {
    temp[i] = char_src[i];
  }
  for (int i = 0; i < num_bytes; i++) {
    char_dest[i] = temp[i];
  }

  free(temp);
  return destination;
}

int memcmp(const void* one, const void* two, uint64_t num_bytes) {
  uint8_t* char_one = (uint8_t*)one;
  uint8_t* char_two = (uint8_t*)two;

  for (int i = 0; i < num_bytes; i++) {
    if (char_one[i] != char_two[i]) {
      return char_one[i] - char_two[i];
    }
  }

  return 0;
}

int strcmp(const void* one, const void* two) {
  uint8_t* char_one = (uint8_t*)one;
  uint8_t* char_two = (uint8_t*)two;

  while (*char_one == *char_two) {
    if (!*char_one || !*char_two) {
      break;
    }
    char_one++;
    char_two++;
  }

  return *char_one - *char_two;
}

char* strcpy(char* dest, const char* src) {
  for (int i = 0; src[i]; i++) {
    dest[i] = src[i];
  }
  return dest;
}

char* strncpy(char* dest, const char* src, int num_bytes) {
  for (int i = 0; i < num_bytes; i++) {
    dest[i] = src[i];
    if (!src[i]) {
      return dest;
    }
  }
  return dest;
}

int strlen(const char* string) {
  int length = 0;
  while (*string++) {
    length++;
  }
  return length;
}

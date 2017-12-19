#ifndef ELF_H_
#define ELF_H_

#include "stdint.h"

struct ELFInfo {
  bool success;
  uint64_t load_address;
  uint64_t num_bytes;
  uint64_t instruction_pointer;

  uint64_t file_offset;
  uint64_t file_size;
};

ELFInfo ELFGetInfo(uint8_t* file, uint64_t filesize);

#endif  // ELF_H_

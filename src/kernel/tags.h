#ifndef TAGS_H_
#define TAGS_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

struct TagsInfo {
  uint64_t region_one_address;
  uint64_t region_one_size;
  uint64_t region_two_address;
  uint64_t region_two_size;

  uint64_t elf_min_address;
  uint64_t elf_max_address;
};

struct TagsInfo ReadTags();

#ifdef __cplusplus
}
#endif

#endif  // TAGS_H_

#ifndef SHARED_CLONE_H_
#define SHARED_CLONE_H_

#include "stdint.h"

struct CloneOptions {
  uint64_t copy_page_table : 1;
  uint64_t start_at_callback : 1;
  uint64_t unused : 62;

  uint64_t Serialize() { return *((uint64_t*)this); }
  static CloneOptions Deserialize(uint64_t options) {
    return *((CloneOptions*)options);
  }
} __attribute__((packed));
static_assert(sizeof(CloneOptions) == sizeof(uint64_t));

typedef void (*CloneCallback)();

void clone(CloneOptions* options, CloneCallback callback, void* new_stack);

#endif  // SHARED_CLONE_H_

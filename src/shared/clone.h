#ifndef SHARED_CLONE_H_
#define SHARED_CLONE_H_

#include "stdint.h"

typedef void (*CloneCallback)();

void clone(CloneCallback callback, void* new_stack);

#endif  // SHARED_CLONE_H_

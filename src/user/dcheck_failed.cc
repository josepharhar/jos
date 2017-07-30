#include "dcheck.h"

#include "printu.h"

void DCHECKFailed(const char* condition) {
  printu("DCHECK failed: %s\n", condition);
}

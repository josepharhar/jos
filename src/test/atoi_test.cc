#undef TEST
#include "shared/stdlib.h"

#include <assert.h>

int main(int argc, char** argv) {
  assert(42 == atoi("42"));
  assert(0 == atoi("0"));
  assert(-2 == atoi("-2"));

  return 0;
}

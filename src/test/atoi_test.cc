#include "test.h"

#include <stdlib.h>

namespace jos {
#include "shared/stdlib.h"
}

int main(int argc, char** argv) {
  ASSERT_EQ(atoi("42"), jos::atoi("42"));
  ASSERT_EQ(atoi("0"), jos::atoi("0"));
  ASSERT_EQ(atoi("-2"), jos::atoi("-2"));

  return 0;
}

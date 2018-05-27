#include "test.h"

#include "shared/stdlib.h"

int main(int argc, char** argv) {
  ASSERT_EQ(42, atoi("42"));
  ASSERT_EQ(0, atoi("0"));
  ASSERT_EQ(-2, atoi(-2));

  return 0;
}

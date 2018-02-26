#ifndef TEST_H_
#define TEST_H_

#include "assert.h"
#include "smartalloc.h"

#include <iostream>

template <typename T>
static void __print_and_assert_eq(std::string string_one,
                                  std::string string_two,
                                  T val_one,
                                  T val_two) {
  std::cout << "Asserting " << string_one << " [" << val_one
            << "] == " << string_two << " [" << val_two << "]" << std::endl;
  assert(val_one == val_two);
}

#define ASSERT_EQ(one, two) __print_and_assert_eq(#one, #two, one, two);

#endif  // TEST_H_

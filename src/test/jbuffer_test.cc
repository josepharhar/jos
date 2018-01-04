#include <string>

#include "test.h"

#include "shared/jbuffer.h"

int main(int argc, char** argv) {
  std::string string_one = "1234";
  std::string string_two = "7654321";
  std::string read_string;
  std::string expected_string;

  stdj::Buffer<char> buffer(20);
  ASSERT_EQ((uint64_t)0, buffer.SizeUsed());

  for (int i = 0; i < 30; i++) {
    ASSERT_EQ((uint64_t)string_one.size(),
              buffer.Write(string_one.data(), string_one.size()));
    ASSERT_EQ((uint64_t)string_two.size(),
              buffer.Write(string_two.data(), string_two.size()));
    expected_string = string_one + string_two;

    ASSERT_EQ((uint64_t)(string_one.size() + string_two.size()),
              buffer.SizeUsed());

    read_string.clear();
    read_string.resize(expected_string.size());
    ASSERT_EQ((uint64_t)read_string.size(),
              buffer.Read((char*)read_string.data(), read_string.size()));

    ASSERT_EQ((uint64_t)0, buffer.SizeUsed());
    ASSERT_EQ(expected_string, read_string);
  }
}

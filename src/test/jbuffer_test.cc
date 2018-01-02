#include <string>

#include "test.h"

int main(int argc, char** argv) {
  std::string string_one = "i am ";
  std::string string_two = "sofa king";
  std::string string_three = "you say funny thing";
  std::string read_string;
  std::string expected_string;

  {
    stdj::Buffer<char> buffer(20);
    buffer.Write(string_one.data(), string_one.size());
    buffer.Write(string_two.data(), string_two.size());
    expected_string = string_one + string_two;

    read_string.clear();
    read_string.resize(expected_string.size());
    buffer.Read(read_string.data(), read_string.size());

    ASSERT_EQ(expected_string, read_string);
  }
}

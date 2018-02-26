#include <string>

#include "test.h"

#include "shared/jbuffer.h"

static void TestBasic() {
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

static void TestOverflow() {
  std::string string_one = "1234";
  std::string string_two = "5678";
  std::string read_string;
  std::string expected_string = "123456";
  expected_string.resize(10);

  stdj::Buffer<char> buffer(6);
  ASSERT_EQ(buffer.Write(string_one.data(), string_one.size()),
            (uint64_t)string_one.size());
  ASSERT_EQ(buffer.Write(string_two.data(), string_two.size()), (uint64_t)2);
  ASSERT_EQ(buffer.SizeUsed(), (uint64_t)6);

  read_string.clear();
  read_string.resize(10);
  ASSERT_EQ(buffer.Read((char*)read_string.data(), read_string.size()),
            (uint64_t)6);
  ASSERT_EQ(buffer.SizeUsed(), (uint64_t)0);
  ASSERT_EQ(read_string, expected_string);
}

static void TestLargeType() {
  stdj::Buffer<uint64_t> buffer(5);

  uint64_t ints[5];
  ints[0] = 100;
  ints[1] = 200;
  ints[2] = 300;
  ints[3] = 400;
  ints[4] = 500;

  uint64_t ints_read[5] = {0};

  buffer.Write(ints, 3);
  buffer.Read(ints_read, 3);
  ASSERT_EQ(ints[0], ints_read[0]);
  ASSERT_EQ(ints[1], ints_read[1]);
  ASSERT_EQ(ints[2], ints_read[2]);

  buffer.Write(ints, 4);
  buffer.Read(ints_read, 4);
  ASSERT_EQ(ints[0], ints_read[0]);
  ASSERT_EQ(ints[1], ints_read[1]);
  ASSERT_EQ(ints[2], ints_read[2]);
  ASSERT_EQ(ints[3], ints_read[3]);
}

int main(int argc, char** argv) {
  TestBasic();
  TestOverflow();
  TestLargeType();
}

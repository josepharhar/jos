#include "test.h"

#include <string>
#include <string.h>

#include "shared/jstring.h"

static bool IsEqual(std::string ref_string, stdj::string j_string) {
  return ref_string == std::string(j_string.Data());
}

static void TestAssignment() {
  const char* hello = "hello";
  std::string ref_string = hello;
  stdj::string j_string = hello;

  assert(IsEqual(ref_string, j_string));
}

static void TestAdd() {
  assert(IsEqual(std::string("one") + std::string("two"),
                 stdj::string("one") + stdj::string("two")));
}

static void TestWeirdLength() {
  const char test_string[] = "/user/init";
  assert(IsEqual(std::string(test_string), stdj::string(test_string)));
}

static void TestSplit() {
  {
    stdj::string string = "/user/init";
    stdj::Array<stdj::string> expected;
    expected.Add("user");
    expected.Add("init");
    stdj::Array<stdj::string> actual = string.Split("/");
    ASSERT_EQ(expected, actual);
  }
  {
    stdj::string string = "/user/init/";
    stdj::Array<stdj::string> expected;
    expected.Add("user");
    expected.Add("init");
    stdj::Array<stdj::string> actual = string.Split("/");
    ASSERT_EQ(expected, actual);
  }
  {
    stdj::string string = "user/init";
    stdj::Array<stdj::string> expected;
    expected.Add("user");
    expected.Add("init");
    stdj::Array<stdj::string> actual = string.Split("/");
    ASSERT_EQ(expected, actual);
  }
  {
    stdj::string string = "asdf";
    stdj::Array<stdj::string> expected;
    stdj::Array<stdj::string> actual = string.Split("asdf");
    ASSERT_EQ(expected, actual);
  }
}

int main(int argc, char** argv) {
  TestAssignment();
  TestAdd();
  TestWeirdLength();
  TestSplit();
}

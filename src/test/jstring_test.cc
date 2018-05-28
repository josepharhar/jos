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

static void TestSubstring() {
  stdj::string string = "asdf";
  ASSERT_EQ(string.Substring(0, 2), stdj::string("as"));
  ASSERT_EQ(string.Substring(1, 2), stdj::string("s"));
  ASSERT_EQ(string.Substring(4, 4), stdj::string(""));
  ASSERT_EQ(string.Substring(0, 4), stdj::string("asdf"));
  ASSERT_EQ(string.Substring(2, 4), stdj::string("df"));
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

void TestParseNumber() {
  ASSERT_EQ(stdj::string("4880"), stdj::string::ParseInt(4880));
  ASSERT_EQ(stdj::string("0"), stdj::string::ParseInt(0));
  ASSERT_EQ(stdj::string("-2"), stdj::string::ParseInt(-2));
  ASSERT_EQ(stdj::string("fead"), stdj::string::ParseInt(0xfead, 16));
}

void TestToInt() {
  ASSERT_EQ((int64_t)1234, stdj::string("1234").ToInt());
  ASSERT_EQ((int64_t)-42, stdj::string("-42").ToInt());
  ASSERT_EQ((int64_t)0, stdj::string("0").ToInt());
  ASSERT_EQ((int64_t)0, stdj::string("0").ToInt(16));
  ASSERT_EQ((int64_t)0xff, stdj::string("ff").ToInt(16));
  ASSERT_EQ((int64_t)0xACDEF, stdj::string("abcdef").ToInt(16));
  ASSERT_EQ((int64_t)0x1a2b3c, stdj::string("0x1A2b3c").ToInt(16));
}

int main(int argc, char** argv) {
  TestAssignment();
  TestAdd();
  TestWeirdLength();
  TestSubstring();
  TestSplit();
  TestParseNumber();
  TestToInt();
}

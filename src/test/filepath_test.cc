#include "test.h"

#include "kernel/vfs/filepath.h"

static void TestParse() {
  vfs::Filepath filepath("/hello/world");
  stdj::Array<stdj::string> actual_strings = filepath.GetArray();

  stdj::Array<stdj::string> expected_strings;
  expected_strings.Add("hello");
  expected_strings.Add("world");

  for (int i = 0; i < actual_strings.Size(); i++) {
    auto expected = expected_strings[i];
    auto actual = actual_strings[i];
    printf("expected_strings[%d]: \"%s\"\n", i, expected.c_str());
    printf("  actual_strings[%d]: \"%s\"\n", i, actual.c_str());
    printf("\"%s\" == \"%s\": %d\n", expected.c_str(), actual.c_str(),
           expected == actual);
    printf("\"%s\".size(): %d, \"%s\".size(): %d\n", expected.c_str(),
           expected.Size(), actual.c_str(), actual.Size());
  }
  if (expected_strings.Size() != actual_strings.Size()) {
    printf("expected size: %d, actual size: %d\n", expected_strings.Size(),
           actual_strings.Size());
    assert(false);
  }

  assert(expected_strings == actual_strings);

  vfs::Filepath filepath_1("hello/");
  stdj::Array<stdj::string> expected_strings_1;
  expected_strings_1.Add("hello");
  printf("asdf: \"%s\"\n", filepath_1.GetArray().Get(0).c_str());
  assert(expected_strings_1 == filepath_1.GetArray());
}

static void TestAppend() {
  vfs::Filepath filepath;
  filepath.Append("hello");
  filepath.Append("world");
  stdj::Array<stdj::string> actual_strings = filepath.GetArray();

  stdj::Array<stdj::string> expected_strings;
  expected_strings.Add("hello");
  expected_strings.Add("world");

  assert(expected_strings == actual_strings);
}

static void TestRemove() {
  vfs::Filepath filepath;
  filepath.Append("one");
  filepath.Append("two");
  filepath.Append("three");
  stdj::Array<stdj::string> expected_strings_1;
  expected_strings_1.Add("one");
  expected_strings_1.Add("two");
  expected_strings_1.Add("three");
  assert(expected_strings_1 == filepath.GetArray());

  assert(stdj::string("one") == filepath.RemoveFirst());
  stdj::Array<stdj::string> expected_strings_2;
  expected_strings_2.Add("two");
  expected_strings_2.Add("three");
  assert(expected_strings_2 == filepath.GetArray());

  assert(stdj::string("three") == filepath.RemoveLast());
  stdj::Array<stdj::string> expected_strings_3;
  expected_strings_3.Add("two");
  assert(expected_strings_3 == filepath.GetArray());

  assert(stdj::string("two") == filepath.RemoveFirst());
  stdj::Array<stdj::string> expected_strings_4;
  assert(expected_strings_4 == filepath.GetArray());
}

static void TestSize() {
  vfs::Filepath filepath;
  ASSERT_EQ(filepath.Size(), 0);

  filepath.Append("hello");
  ASSERT_EQ(filepath.Size(), 1);

  filepath.Append("world");
  ASSERT_EQ(filepath.Size(), 2);

  filepath.RemoveFirst();
  ASSERT_EQ(filepath.Size(), 1);

  filepath.RemoveLast();
  ASSERT_EQ(filepath.Size(), 0);
}

int main(int argc, char** argv) {
  TestAppend();
  TestParse();
  TestRemove();
  TestSize();
}

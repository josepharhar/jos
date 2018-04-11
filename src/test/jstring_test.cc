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

int main(int argc, char** argv) {
  TestAssignment();
  TestAdd();
}

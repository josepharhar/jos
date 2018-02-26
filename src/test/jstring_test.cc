#include "test.h"

#include <string>
#include <string.h>

#include "shared/jstring.h"

bool IsEqual(std::string ref_string, stdj::string j_string) {
  return ref_string == std::string(j_string.Data());
}

void TestAssignment() {
  std::string ref_string = "hello";
  stdj::string j_string = "hello";

  assert(IsEqual(ref_string, j_string));
}

int main(int argc, char** argv) {
  TestAssignment();
}

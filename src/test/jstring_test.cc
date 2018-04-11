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

  printf("   literal: 0x%016lX\n", (uint64_t)hello);
  printf("ref_string: 0x%016lX\n", (uint64_t)ref_string.c_str());
  printf("  j_string: 0x%016lX\n", (uint64_t)j_string.c_str());

  //printf(" malloc(7): 0x%016lX\n", (uint64_t)malloc(7));

  /*printf("ref_string: \"%s\"\n", ref_string.c_str());
  printf("  j_string: \"%s\"\n", j_string.c_str());

  printf("modifying j_string...\n");
  *((char*)j_string.c_str()) = 'j';

  printf("ref_string: 0x%016lX\n", (uint64_t)ref_string.c_str());
  printf("  j_string: 0x%016lX\n", (uint64_t)j_string.c_str());

  printf("ref_string: \"%s\"\n", ref_string.c_str());
  printf("  j_string: \"%s\"\n", j_string.c_str());*/

  //assert(IsEqual(ref_string, j_string));
}

static void TestAdd() {
  assert(IsEqual(std::string("one") + std::string("two"),
                 stdj::string("one") + stdj::string("two")));
}

int main(int argc, char** argv) {
  //TestAssignment();
  //TestAdd();
}

#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <vector>

#include "test.h"

#include "shared/jarray.h"

int main(int argc, char** argv) {
  int one = 1;
  //while (one);
  stdj::Array<char> jarray;
  jarray.Add('j');
  jarray.Add('o');
  jarray.Add('s');
  assert(!strcmp(jarray.Data(), "jos"));
}

#include <assert.h>

#include <iostream>
#include <vector>

#include "shared/jarray.h"

static bool IsEqual(std::vector<int> vector, stdj::Array<int> array) {
  if (vector.size() != array.Size()) {
    return false;
  }
  for (int i = 0; i < vector.size(); i++) {
    if (vector[i] != array.Get(i)) {
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  // TODO

  std::vector<int> vector;
  stdj::Array<int> array;
  assert(IsEqual(vector, array));

  vector.push_back(1);
  array.Add(1);
  assert(IsEqual(vector, array));

  vector.push_back(10);
  array.Add(10);
  assert(IsEqual(vector, array));

  vector.push_back(100);
  array.Add(100);
  assert(IsEqual(vector, array));

  vector.erase(vector.begin() + 1);
  array.Remove(1);
  assert(IsEqual(vector, array));

  vector.erase(vector.begin() + 1);
  array.Remove(1);
  assert(IsEqual(vector, array));

  vector.erase(vector.begin() + 0);
  array.Remove(0);
  assert(IsEqual(vector, array));

  return 0;
}

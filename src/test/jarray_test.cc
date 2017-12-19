#include <assert.h>

#include <iostream>
#include <vector>

#include "test.h"

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

void TestBasicVector() {
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
}

void TestGetNext() {
  stdj::Array<int> array;
  array.Add(1);

  ASSERT_EQ(array.GetNextValue(1), 1);
  ASSERT_EQ(array.GetPreviousValue(1), 1);

  array.Add(10);
  ASSERT_EQ(array.GetNextValue(1), 10);
  ASSERT_EQ(array.GetNextValue(10), 1);
  ASSERT_EQ(array.GetPreviousValue(1), 10);
  ASSERT_EQ(array.GetPreviousValue(10), 1);

  array.Add(100);
  ASSERT_EQ(array.GetNextValue(1), 10);
  ASSERT_EQ(array.GetNextValue(10), 100);
  ASSERT_EQ(array.GetNextValue(100), 1);
  ASSERT_EQ(array.GetPreviousValue(1), 100);
  ASSERT_EQ(array.GetPreviousValue(10), 1);
  ASSERT_EQ(array.GetPreviousValue(100), 10);
}

int main(int argc, char** argv) {
  TestBasicVector();
  TestGetNext();

  return 0;
}

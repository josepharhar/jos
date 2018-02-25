#include "test.h"

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
  array.RemoveAt(1);
  assert(IsEqual(vector, array));

  vector.erase(vector.begin() + 1);
  array.RemoveAt(1);
  assert(IsEqual(vector, array));

  vector.erase(vector.begin() + 0);
  array.RemoveAt(0);
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

void TestCopying() {
  stdj::Array<int> array;
  array.Add(50);
  array.Add(40);
  array.Add(30);

  std::vector<int> vector;
  vector.push_back(50);
  vector.push_back(40);
  vector.push_back(30);

  assert(IsEqual(vector, array));

  stdj::Array<int> copy = array;
  std::vector<int> vector_copy = vector;
  array.RemoveAt(0);
  vector.erase(vector.begin());

  assert(IsEqual(vector, array));
  assert(IsEqual(vector_copy, copy));

  stdj::Array<int> copy2(array);
  std::vector<int> vector_copy2(vector);
  array.RemoveAt(0);
  vector.erase(vector.begin());

  assert(IsEqual(vector, array));
  assert(IsEqual(vector_copy, copy));
  assert(IsEqual(vector_copy2, copy2));
}

int main(int argc, char** argv) {
  TestBasicVector();
  TestGetNext();
  TestCopying();

  return 0;
}

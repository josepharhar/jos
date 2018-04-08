#include <assert.h>

#include <iostream>
#include <vector>

#include "test.h"

#include "shared/jarray.h"

static bool IsEqual(std::vector<int> vector, stdj::Array<int> array) {
  if (vector.size() != array.Size()) {
    printf("IsEqual() vector.size(): %d, array.Size(): %d\n", vector.size(),
           array.Size());
    return false;
  }
  for (int i = 0; i < vector.size(); i++) {
    if (vector[i] != array.Get(i)) {
      printf("IsEqual() vector[%d]: %d, array.Get(%d): %d\n", i, vector[i], i,
             array.Get(i));
      return false;
    }
  }
  return true;
}

static void TestBasicVector() {
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

static void TestGetNext() {
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

static void TestCopying() {
  stdj::Array<int> array;
  array.Add(50);
  array.Add(40);
  array.Add(30);
  array.Add(20);

  std::vector<int> vector;
  vector.push_back(50);
  vector.push_back(40);
  vector.push_back(30);
  vector.push_back(20);

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

void TestSubstring() {
  stdj::Array<char> j_array;
  j_array.Add('j');
  j_array.Add('o');
  j_array.Add('s');

  stdj::Array<char> expected_substring;
  expected_substring.Add('o');
  assert(expected_substring == j_array.Substring(1, 2));

  stdj::Array<char> expected_substring_2;
  expected_substring_2.Add('j');
  expected_substring_2.Add('o');
  assert(expected_substring_2 == j_array.Substring(0, 2));

  stdj::Array<char> expected_substring_3;
  expected_substring_3.Add('o');
  expected_substring_3.Add('s');
  assert(expected_substring_3 == j_array.Substring(1, 3));

  assert(j_array == j_array.Substring(0, 3));
}

static void TestAdd() {
  stdj::Array<char> array_one;
  array_one.Add('a');
  array_one.Add('s');

  stdj::Array<char> array_two;
  array_two.Add('d');
  array_two.Add('f');

  stdj::Array<char> expected;
  expected.Add('a');
  expected.Add('s');
  expected.Add('d');
  expected.Add('f');

  assert(expected == array_one + array_two);
}

static void TestSize() {
  stdj::Array<uint64_t> array;
  assert(array.Size() == 0);

  array.Add(1234);
  assert(array.Size() == 1);

  array.Add(1234);
  assert(array.Size() == 2);
}

int main(int argc, char** argv) {
  TestBasicVector();
  TestGetNext();
  TestCopying();
  TestSubstring();
  TestAdd();

  return 0;
}

#include <assert.h>

#include <iostream>
#include <vector>

#include "shared/linked_list.h"

static bool IsEqual(std::vector<int> vector, LinkedList<int> list) {
  if (vector.size() != list.Size()) {
    return false;
  }

  for (int i = 0; i < vector.size(); i++) {
    if (vector[i] != list.Get(i)) {
      return false;
    }
  }

  return true;
}

int main(int argc, char** argv) {
  std::vector<int> numbers;
  numbers.push_back(1);
  numbers.push_back(2);
  numbers.push_back(3);

  LinkedList<int> list;

  for (int i = 0; i < numbers.size(); i++) {
    list.Add(numbers[i]);
  }

  assert(IsEqual(numbers, list));

  return 0;
}

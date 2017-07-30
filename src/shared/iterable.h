#ifndef ITERABLE_H_
#define ITERABLE_H_

#include "iterator.h"

template <typename T>
class Iterable {
 public:
  Iterable() {}
  virtual ~Iterable() {}

  virtual Iterator<T>* GetIterator() = 0;
};

#endif  // ITERABLE_H_

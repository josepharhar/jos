#ifndef COLLECTION_H_
#define COLLECTION_H_

#include "iterable.h"

template <typename T>
class Collection : public Iterable<T> {
 public:
  Collection() {}
  virtual ~Collection() {}

  virtual bool Add(T value) = 0;
  virtual bool Remove(T value) = 0;

  bool Contains(T value) {
    Iterator<E> iterator = Iterator();
    while (iterator.HasNext()) {
      if (iterator.Next() == value) {
        return true;
      }
    }
    return false;
  }

  uint64_t Size() {
    uint64_t size = 0;
    Iterator<E> iterator = Iterator();
    while (iterator.HasNext()) {
      iterator.Next();
      size++;
    }
    return size;
  }
};

#endif  // COLLECTION_H_

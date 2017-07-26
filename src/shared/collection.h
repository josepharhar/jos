#ifndef COLLECTION_H_
#define COLLECTION_H_

#include "iterable.h"

template <typename T>
class Collection : public Iterable<T> {
 public:
  Collection() {}
  virtual ~Collection() {}

  virtual Iterator<T>* GetIterator() = 0;

  bool Contains(T value) {
    Iterator<T>* iterator = GetIterator();
    while (iterator->HasNext()) {
      if (iterator->Next() == value) {
        delete iterator;
        return true;
      }
    }
    delete iterator;
    return false;
  }

  uint64_t Size() {
    uint64_t size = 0;
    Iterator<T>* iterator = GetIterator();
    while (iterator->HasNext()) {
      iterator->Next();
      size++;
    }
    delete iterator;
    return size;
  }

  bool IsEmpty() {
    return !Size();
  }
};

#endif  // COLLECTION_H_

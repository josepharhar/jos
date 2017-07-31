#ifndef COLLECTION_H_
#define COLLECTION_H_

#include "iterable.h"

void printk(...);

template <typename T>
class Collection : public Iterable<T> {
 public:
  Collection() {}
  virtual ~Collection() {}

  bool Contains(T value) {
    Iterator<T>* iterator = this->GetIterator();
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
    Iterator<T>* iterator = this->GetIterator();
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

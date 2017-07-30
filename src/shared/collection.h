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
    printk("1\n");
    uint64_t size = 0;
    printk("this: %p\n", this);
    //printk("this->GetIterator(): %p\n", this->GetIterator);
    Iterator<T>* iterator = this->GetIterator();
    printk("2\n");
    while (iterator->HasNext()) {
      iterator->Next();
      size++;
    }
    printk("3\n");
    delete iterator;
    printk("4\n");
    return size;
  }

  bool IsEmpty() {
    printk("isempty()\n");
    return !Size();
  }
};

#endif  // COLLECTION_H_

#ifndef LINKED_SET_H_
#define LINKED_SET_H_

#include "set.h"

#include "linked_list.h"

template <typename T>
class LinkedSet : public Set<T> {
 public:
  LinkedSet() {}
  ~LinkedSet() {}

  void Add(T value) override {
    Iterator<T>* iterator = GetIterator();
    while (iterator->HasNext()) {
      if (iterator->Next() == value) {
        delete iterator;
        return;
      }
    }
    linked_list_.Add(value);
    delete iterator;
  }

  bool Remove(T value) override {
    linked_list_.Remove(value);
  }

  Iterator<T>* Iterator() override {
    return linked_list_.Iterator();
  }

 private:
  LinkedList<T> linked_list_;
};

#endif  // LINKED_SET_H_

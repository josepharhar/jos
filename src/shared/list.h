#ifndef LIST_H_
#define LIST_H_

#include "collection.h"
#include "dcheck.h"

template <typename T>
class List : public Collection<T> {
 public:
  List() {}
  virtual ~List() {}

  virtual void Add(T value) = 0;
  virtual bool Remove(T value) = 0;

  virtual Iterator<T>* GetIterator() = 0;

  T Get(int index) {
    DCHECK_MESSAGE(index < this->Size() && index >= 0, "index: %d", index);

    Iterator<T>* iterator = GetIterator();
    for (int i = 0; i < index; i++) {
      iterator->Next();
    }

    T value = iterator->Next();
    delete iterator;
    return value;
  }

  int GetIndex(T value) {
    Iterator<T>* iterator = GetIterator();
    int i = 0;
    while (iterator->HasNext()) {
      if (iterator->Next() == value) {
        delete iterator;
        return i;
      }
      i++;
    }
    delete iterator;
    return -1;
  }

  T GetNext(T value) {
    int index = GetIndex(value);
    if (index == this->Size() - 1) {
      return Get(0);
    }
    return Get(index + 1);
  }
  T GetPrevious(T value) {
    int index = GetIndex(value);
    if (index == 0) {
      return Get(this->Size() - 1);
    }
    return Get(index - 1);
  }
};

#endif  // LIST_H_

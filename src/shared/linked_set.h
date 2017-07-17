#ifndef LINKED_SET_H_
#define LINKED_SET_H_

#include "set.h"

template <typename T>
class LinkedSet : public Set<T> {
 public:
  LinkedSet() {}
  ~LinkedSet() {}

 private:
  template <typename T>
  struct SetEntry {

  };
};

#endif  // LINKED_SET_H_

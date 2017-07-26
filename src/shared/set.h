#ifndef SET_H_
#define SET_H_

#include "collection.h"

template <typename T>
class Set : public Collection<T> {
 public:
  Set() {}
  virtual ~Set() {}

  virtual void Add(T value) = 0;
  virtual bool Remove(T value) = 0;
};

#endif  // SET_H_

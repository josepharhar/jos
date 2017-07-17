#ifndef SET_H_
#define SET_H_

#include "collection.h"

template <typename T>
class Set : public Collection<T> {
 public:
  Set() {}
  virtual ~Set() {}
};

#endif  // SET_H_

#ifndef COLLECTION_H_
#define COLLECTION_H_

#include "iterable.h"

template <typename T>
class Collection : public Iterable<T> {
 public:
  Collection() {}
  virtual ~Collection() {}

  virtual bool Add(T element) = 0;
  virtual bool Contains(T element) = 0;
  virtual bool Remove(T element) = 0;
  virtual uint64_t Size() = 0;
};

#endif  // COLLECTION_H_

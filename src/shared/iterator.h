#ifndef ITERATOR_H_
#define ITERATOR_H_

template <typename T>
class Iterator {
 public:
  Iterator() {}
  virtual ~Iterator() {}

  virtual T Next() = 0;
  virtual bool HasNext() = 0;
};

#endif  // ITERATOR_H_

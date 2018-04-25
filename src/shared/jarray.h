#ifndef SHARED_JARRAY_H_
#define SHARED_JARRAY_H_

#include "jarray_base.h"

namespace stdj {

template <typename T>
class Array : public ArrayBase<T> {
 public:
  Array(T* new_array, int new_array_size)
      : array_(new T[new_array_size]),
        array_size_(new_array_size),
        size_(new_array_size) {
    memcpy(array_, new_array, new_array_size * sizeof(T));
    memset(array_ + new_array_size, 0, sizeof(T));
  }

  ~Array() {
    if (array_) {
      delete[] array_;
    }
  }

  void Add(T value) override {
    // increase array size if needed
    if (size_ + 1 >= array_size_) {
      uint64_t old_array_size = array_size_;
      T* old_array = array_;

      array_size_ = (old_array_size + 5) * 2;
      array_ = new T[array_size_];
      memset(array_, 0, array_size_ * sizeof(T));

      if (old_array) {
        memcpy(array_, old_array, old_array_size * sizeof(T));
        delete[] old_array;
      }
    }

    array_[size_] = value;
    size_++;
  }

 protected:
  void CopyFrom(const ArrayBase<T>& other) {
    size_ = other.size_;
    array_size_ = other.array_size_;
    array_ = 0;
    if (array_size_) {
      array_ = new T[array_size_];
      for (int i = 0; i < size_; i++) {
        array_[i] = other.array_[i];
      }
      // memset(array_ + size_, 0, (array_size_ - size_) * sizeof(T));
    }
  }
};

}  // namespace stdj

#endif  // SHARED_JARRAY_H_

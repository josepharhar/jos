#ifndef SHARED_ARRAY_H_
#define SHARED_ARRAY_H_

#include "string.h"

template <typename T>
class Array {
 public:
  Array() : array_(0), array_size_(0), size_(0) {}
  ~Array() {
    if (array_) {
      free(array_);
    }
  }

  Array<T>(const Array<T>& other) = delete;
  Array<T>& operator=(const Array<T>& other) = delete;

  Array<T>(Array<T>&& other) = delete;
  Array<T>& operator=(Array<T>&& other) = delete;

  T Get(uint64_t index) {
    if (index >= size_) {
      // TODO assert
    }
    return array_[index];
  }

  T* Data() {
    return array_;
  }

  void Add(T value) {
    if (size_ + 1 > array_size_) {
      // increase array size
      uint64_t old_array_size = array_size_;
      T* old_array = array_;

      array_size_ = (old_array_size + 5) * 2;

      array_ = malloc(array_size_ * sizeof(T));
      memcpy(array_, old_array, old_array_size * sizeof(T));

      if (old_array) {
        free(old_array);
      }
    }

    array_[size_] = value;
    size_++;
  }

  void Remove(uint64_t index) {
    if (index >= size_) {
      // TODO assert
    }
    memmove(array_ + index, array_ + index + 1, size_ - index);
    size_--;
  }

  uint64_t Size() const { return size_; }

 private:
  T* array_;
  // actual size
  uint64_t array_size_;

  // advertized size
  uint64_t size_;
};

#endif  // SHARED_ARRAY_H_

#ifndef SHARED_JARRAY_BASE_H_
#define SHARED_JARRAY_BASE_H_

#ifdef TEST
#include <stdio.h>
#include <iostream>
#include <ostream>
#endif

#include "string.h"
#include "dcheck.h"

namespace stdj {

template <typename T>
class Array {
 public:
  ArrayBase() : array_(0), array_size_(0), size_(0) {}

  /*~ArrayBase() {
    if (array_) {
      delete[] array_;
    }
  }*/

  ArrayBase<T>(const ArrayBase<T>& other) { CopyFrom(other); }
  ArrayBase<T>& operator=(const ArrayBase<T>& other) {
    CopyFrom(other);
    return *this;
  }

  bool Equals(const ArrayBase<T>& other) const {
    if (size_ != other.size_) {
      return false;
    }
    for (int i = 0; i < size_; i++) {
      if (array_[i] != other.array_[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator==(const ArrayBase<T>& other) const { return Equals(other); }
  bool operator!=(const ArrayBase<T>& other) const { return !Equals(other); }

  T Get(uint64_t index) const {
    if (index >= size_) {
      // TODO assert
    }
    return array_[index];
  }
  T operator[](int index) { return Get(index); }

  T* Data() const { return array_; }

  virtual void Add(T value) = 0;

  T RemoveAt(uint64_t index) {
    T value = Get(index);
    if (index >= size_) {
      // TODO assert
    }
    memmove(array_ + index, array_ + index + 1, (size_ - index) * sizeof(T));
    memset(array_ + size_, 0, sizeof(T));
    size_--;
    return value;
  }

  void RemoveValue(T value) {
    int index = GetIndexOfValue(value);
    if (index != -1) {
      RemoveAt(index);
    }
  }

  uint64_t Size() const { return size_; }

  bool IsEmpty() const { return Size() == 0; }

  // Gets the next value in the list given a value, and loops around to the
  // front of the list when it hits the end.
  T GetNextValue(T value) const {
    int value_index = GetIndexOfValue(value);
    DCHECK(value_index != -1);
    value_index++;
    if (value_index >= size_) {
      value_index = 0;
    }
    return array_[value_index];
  }

  T GetPreviousValue(T value) const {
    int value_index = GetIndexOfValue(value);
    DCHECK(value_index != -1);
    value_index--;
    if (value_index == -1) {
      value_index = size_ - 1;
    }
    return array_[value_index];
  }

  int GetIndexOfValue(T value) const {
    for (int i = 0; i < size_; i++) {
      if (array_[i] == value) {
        return i;
      }
    }
    return -1;
  }

  bool Contains(T value) const { return GetIndexOfValue(value) != -1; }

  ArrayBase<T> Substring(int one, int two) {
    ArrayBase<T> new_array;
    if (one >= two) {
      return new_array;
    }
    new_array.size_ = two - one;
    new_array.array_size_ = new_array.size_ + 1;
    new_array.array_ = new T[new_array.array_size_];
    memcpy(new_array.array_, array_ + one, new_array.size_ * sizeof(T));
    return new_array;
  }

  ArrayBase<T> Substring(int one) { return Substring(one, Size()); }

  ArrayBase<T> operator+(const ArrayBase<T>& other) {
    ArrayBase<T> new_array;
    for (unsigned i = 0; i < Size(); i++) {
      new_array.Add(Get(i));
    }
    for (unsigned i = 0; i < other.Size(); i++) {
      new_array.Add(other.Get(i));
    }
    return new_array;
  }

#ifdef TEST
  friend std::ostream& operator<<(std::ostream& os, const ArrayBase<T> array) {
    os << "[";
    for (int i = 0; i < array.Size(); i++) {
      os << array.Get(i);
      if (i != array.Size() - 1) {
        os << ",";
      }
    }
    os << "]";
    return os;
  }
#endif

 protected:
  T* array_;
  uint64_t array_size_;  // actual size
  uint64_t size_;        // advertized size

  virtual void CopyFrom(const ArrayBase<T>& other) = 0;
};

}  // namespace stdj

#endif  // SHARED_JARRAY_BASE_H_

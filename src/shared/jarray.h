#ifndef SHARED_JARRAY_H_
#define SHARED_JARRAY_H_

#include "string.h"
#include "dcheck.h"

namespace stdj {

template <typename T>
class Array {
 public:
  Array() : array_(0), array_size_(0), size_(0) {}
  Array(T* new_array, int new_array_size)
      : array_(new T[new_array_size + 1]),
        array_size_(new_array_size + 1),
        size_(new_array_size) {
    memcpy(array_, new_array, new_array_size * sizeof(T));
    memset(array_ + new_array_size, 0, sizeof(T));
  }

  virtual ~Array() {
    if (array_) {
      delete[] array_;
    }
  }

  Array<T>(const Array<T>& other) { CopyFrom(other); }
  Array<T>& operator=(const Array<T>& other) {
    CopyFrom(other);
    return *this;
  }

  Array<T>(Array<T>&& other) = default;
  Array<T>& operator=(Array<T>&& other) = default;

  bool Equals(const Array<T>& other) const {
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
  bool operator==(const Array<T>& other) const { return Equals(other); }
  bool operator!=(const Array<T>& other) const { return !Equals(other); }

  T Get(uint64_t index) const {
    if (index >= size_) {
      // TODO assert
    }
    return array_[index];
  }
  T operator[](int index) { return Get(index); }

  T* Data() const { return array_; }

  void Add(T value) {
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

  void RemoveAt(uint64_t index) {
    if (index >= size_) {
      // TODO assert
    }
    memmove(array_ + index, array_ + index + 1, (size_ - index) * sizeof(T));
    memset(array_ + size_, 0, sizeof(T));
    size_--;
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

  Array<T> Substring(int one, int two) {
    Array<T> new_array;
    if (one >= two) {
      return new_array;
    }
    new_array.size_ = two - one;
    new_array.array_size_ = new_array.size_ + 1;
    new_array.array_ = new T[new_array.array_size_];
    memcpy(new_array.array_, array_ + one, new_array.size_ * sizeof(T));
    return new_array;
  }

  Array<T> Substring(int one) { return Substring(one, Size()); }

  Array<T> operator+(const Array<T>& other) {
    Array<T> new_array;
    for (unsigned i = 0; i < Size(); i++) {
      new_array.Add(Get(i));
    }
    for (unsigned i = 0; i < other.Size(); i++) {
      new_array.Add(other.Get(i));
    }
    return new_array;
  }

 private:
  T* array_;
  // actual size
  uint64_t array_size_;

  // advertized size
  uint64_t size_;

  void CopyFrom(const Array<T>& other) {
    size_ = other.size_;
    array_size_ = other.array_size_;
    array_ = 0;
    if (array_size_) {
      array_ = new T[array_size_];
      memcpy(array_, other.array_, array_size_ * sizeof(T));
    }
  }
};

}  // namespace stdj

#endif  // SHARED_JARRAY_H_

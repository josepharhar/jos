#ifndef SHARED_ARRAY_H_
#define SHARED_ARRAY_H_

template <typename T>
class Array {
 public:
  Array() : array_(0), array_size_(0), size_(0) {}
  ~Array() {
    if (array_) {
      free(array_);
    }
  }

  Array(const Array& other) = delete;
  Array& operator=(const Array& other) = delete;

  Array(Array&& other) = delete;
  Array& operator=(Array&& other) = delete;

  T* Get(uint64_t index) {
    if (index >= array_size_) {
      // TODO assert
    }
    return array_ + index;
  }

  void Add(T value) {
    if (size_ + 1 > array_size_) {
      // increase array size
      uint64_t old_array_size_ = array_size_;
      T* old_array_ = array_;

      array_size_ = (old_array_size + 5) * 2;

      array_ = malloc(array_size_ * sizeof(T));
      memcpy(array_, old_array_, old_array_size_ * sizeof(T));
      /*array_ = new T[array_size_];
      for (uint64_t i = 0; i < old_array_size_; i++) {
        array_[i] = old_array_[i];
      }*/

      if (array_) {
        free(array_);
      }
    }
  }

  void Remove(uint64_t index) {}

  uint64_t Size() const {
    return size_;
  }

 private:
  T* array_;
  // actual size
  uint64_t array_size_;

  // advertized size
  uint64_t size_;
};

#endif  // SHARED_ARRAY_H_

#ifndef SHARED_JBUFFER_H_
#define SHARED_JBUFFER_H_

#include "string.h"

namespace stdj {

// Stores |size| "T"s in a circular buffer
template <typename T>
class Buffer {
 public:
  Buffer(uint64_t size)
      : size_(size),
        read_index_(0),
        write_index_(0),
        buffer_(new T[size]),
        is_full_(false) {}
  ~Buffer() { delete[] buffer_; }

  Buffer(const Buffer& other) = delete;
  Buffer& operator=(const Buffer& other) = delete;

  Buffer(Buffer&& other) = delete;
  Buffer& operator=(Buffer&& other) = delete;

  // Returns number of entries written
  uint64_t Write(const T* source_buffer, uint64_t write_size) {
    if (SizeRemaining() < write_size) {
      write_size = SizeRemaining();
    }

    // check to see if we have to do multiple copies
    if (write_index_ + write_size >= Size()) {
      // multiple copies
      uint64_t first_copy_size = Size() - write_index_;
      memcpy(buffer_ + write_index_, source_buffer, first_copy_size);
      memcpy(buffer_, source_buffer + first_copy_size,
             write_size - first_copy_size);
      write_index_ = write_size - first_copy_size;

    } else {
      // one copy
      memcpy(buffer_ + write_index_, source_buffer, write_size);
      write_index_ = (write_index_ + write_size) % size_;
    }

    // update is_full_
    if (write_size > 0 && write_index_ == read_index_) {
      is_full_ = true;
    } else {
      is_full_ = false;
    }

    return write_size;
  }

  // Returns number of entries read
  uint64_t Read(T* dest_buffer, uint64_t read_size) {
    if (read_size > SizeUsed()) {
      read_size = SizeUsed();
    }

    if (read_index_ + read_size >= Size()) {
      // two copies
      uint64_t first_copy_size = Size() - read_index_;
      memcpy(dest_buffer, buffer_ + read_index_, first_copy_size);
      memcpy(dest_buffer + first_copy_size, buffer_,
             (read_size - first_copy_size));
      read_index_ = read_size - first_copy_size;

    } else {
      // one copy
      memcpy(dest_buffer, buffer_ + read_index_, read_size);
      read_index_ = (read_index_ + read_size) % size_;
    }

    // update is_full_
    if (read_size > 0 && read_index_ == write_index_) {
      is_full_ = false;
    } else {
      is_full_ = true;
    }

    return read_size;
  }

  uint64_t Size() const { return size_; }

  uint64_t SizeUsed() const {
    if (read_index_ == write_index_) {
      return is_full_ ? size_ : 0;
    } else if (write_index_ > read_index_) {
      return write_index_ - read_index_;
    } else {
      return (Size() - read_index_) + write_index_;
    }
  }
  uint64_t ReadSizeAvailable() const { return SizeUsed(); }

  uint64_t SizeRemaining() const { return Size() - SizeUsed(); }
  uint64_t WriteSizeAvailable() const { return SizeRemaining(); }

 private:
  T* buffer_;
  uint64_t size_;
  uint64_t read_index_;
  uint64_t write_index_;
  bool is_full_;
};

}  // namespace stdj

#endif  // SHARED_JBUFFER_H_

#ifndef SHARED_JBUFFER_H_
#define SHARED_JBUFFER_H_

namespace stdj {

template <typename T>
class Buffer {
 public:
  Buffer(uint64_t size)
      : size_(size), read_index_(0), write_index_(1), buffer_(new T[size]) {}
  ~Buffer() { delete[] buffer_; }

  Buffer(const Buffer& other) = delete;
  Buffer& operator=(const Buffer& other) = delete;

  Buffer(Buffer&& other) = delete;
  Buffer& operator=(Buffer&& other) = delete;

  void Write(const T* source_buffer, uint64_t write_size) {
  }

  void Read(T* dest_buffer, uint64_t read_size) {
  }

  uint64_t Size() const { return size_; }

 private:
  T* buffer_;
  uint64_t size_;
  uint64_t read_index_;
  uint64_t write_index_;
};

}  // namespace stdj

#endif  // SHARED_JBUFFER_H_

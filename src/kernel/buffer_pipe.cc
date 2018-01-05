#include "buffer_pipe.h"

BufferPipe::BufferPipe() : buffer_(4096) {}
BufferPipe::~BufferPipe() {}

int BufferPipe::Write(const uint8_t* source_buffer, int write_size) {
  return (int)buffer_.Write(source_buffer, (uint64_t)write_size);
}

int BufferPipe::Read(uint8_t* dest_buffer, int read_size) {
  return (int)buffer_.Read(dest_buffer, (uint64_t)read_size);
}

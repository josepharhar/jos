#ifndef BUFFER_PIPE_H_
#define BUFFER_PIPE_H_

#include "pipe.h"
#include "shared/jbuffer.h"

class BufferPipe : public Pipe {
 public:
  BufferPipe();
  ~BufferPipe();

  BufferPipe(const BufferPipe& other) = delete;
  BufferPipe& operator=(const BufferPipe& other) = delete;

  BufferPipe(BufferPipe&& other) = delete;
  BufferPipe& operator=(BufferPipe&& other) = delete;

  int Write(const uint8_t* source_buffer, int write_size) override;
  int Read(uint8_t* dest_buffer, int read_size) override;

 private:
  stdj::Buffer<uint8_t> buffer_;
};

#endif  // BUFFER_PIPE_H_

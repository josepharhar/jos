#ifndef BUFFER_FILE_H_
#define BUFFER_FILE_H_

#include "ipc.h"
#include "kernel/proc.h"
#include "shared/jbuffer.h"
#include "shared/jarray.h"

class BufferFile : public ipc::File {
 public:
  BufferFile();
  ~BufferFile() override;

  BufferFile(const BufferFile& other) = delete;
  BufferFile& operator=(const BufferFile& other) = delete;
  BufferFile(BufferFile&& other) = delete;
  BufferFile& operator=(BufferFile&& other) = delete;

  ipc::Pipe* Open(ipc::Mode mode) override;
  void Close(ipc::Pipe* pipe) override;
  int GetNumPipes() override;

 private:
  struct RdWrRequest {
    Pipe* pipe;
    uint8_t* buffer;
    int size;
  };

  stdj::Array<ipc::Pipe*> pipes_;
  stdj::Buffer<uint8_t> buffer_;

  proc::BlockedQueue write_blocked_queue_;
  stdj::Queue<RdWrRequest> write_request_queue_;

  proc::BlockedQueue read_blocked_queue_;
  stdj::Queue<RdWrRequest> read_request_queue_;
};

class BufferPipe : public ipc::Pipe {
 public:
  BufferPipe(ipc::File* file, ipc::Mode mode);
  ~BufferPipe() override;

  BufferPipe(const BufferPipe& other) = delete;
  BufferPipe& operator=(const BufferPipe& other) = delete;
  BufferPipe(BufferPipe&& other) = delete;
  BufferPipe& operator=(BufferPipe&& other) = delete;

  int Write(const uint8_t* source_buffer, int write_size) override;
  int Read(uint8_t* dest_buffer, int read_size) override;
};

#endif  // BUFFER_FILE_H_

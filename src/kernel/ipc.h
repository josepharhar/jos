#ifndef KERNEL_IPC_H_
#define KERNEL_IPC_H_

#include "stdint.h"

namespace proc {
class BlockedQueue;
}

namespace ipc {

enum Mode {
  RDONLY = 1,
  WRONLY = 2,
  RDWR = 3,
};

class Pipe;

class File {
 public:
  File();
  virtual ~File();

  // Opens a pipe to this file in the specified mode.
  virtual Pipe* Open(Mode mode) = 0;
  // Closes and deletes a pipe created using Open().
  virtual void Close(Pipe* pipe) = 0;
  // Gets the number of pipes currently open accessing this file.
  virtual int GetNumPipes() = 0;
  // Reads or writes to the underlying file. Will block the current process if
  // there is no data to be read or written. I don't know why I added the pipe
  // parameter. Returns -1 on error, RDWR_BLOCKED on blocking
  virtual void Write(Pipe* pipe,
                     const uint8_t* source_buffer,
                     int write_buffer,
                     int* size_writeback) = 0;
  virtual void Read(Pipe* pipe,
                    uint8_t* dest_buffer,
                    int read_size,
                    int* size_writeback) = 0;
};

// one ipc::Pipe per file descriptor per process
class Pipe {
 public:
  Pipe(File* file, Mode mode);
  virtual ~Pipe();

  // these pass through to ipc::File::Write/Read
  void Write(const uint8_t* source_buffer, int write_size, int* size_writeback);
  void Read(uint8_t* dest_buffer, int read_size, int* size_writeback);

  // Returns the ipc::File which owns this Pipe
  File* GetFile();

  Mode GetMode();

 private:
  File* file_;
  Mode mode_;
};

}  // namespace ipc

#endif  // KERNEL_IPC_H_

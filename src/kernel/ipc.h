#ifndef KERNEL_IPC_H_
#define KERNEL_IPC_H_

#include "stdint.h"

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
};

class Pipe {
 public:
  Pipe(File* file, Mode mode);
  virtual ~Pipe();

  virtual int Write(const uint8_t* source_buffer, int write_size) = 0;
  virtual int Read(uint8_t* dest_buffer, int read_size) = 0;

  // Returns the ipc::File which owns this Pipe
  File* GetFile();

  Mode GetMode();

 private:
  File* file_;
  Mode mode_;
};

}  // namespace ipc

#endif  // KERNEL_IPC_H_

#include "ipc.h"

namespace ipc {

File::File() {}

Pipe::Pipe(File* file, Mode mode) : file_(file), mode_(mode), closed_(false) {}

File* Pipe::GetFile() {
  return file_;
}
Mode Pipe::GetMode() {
  return mode_;
}

void Pipe::Write(const uint8_t* source_buffer,
                 int write_size,
                 int* size_writeback) {
  if (closed_) {
    // assuming calling proc's page table is in cr3
    // TODO what is supposed to happen when you write to a closed pipe?
    *size_writeback = 0;
  } else {
    GetFile()->Write(this, source_buffer, write_size, size_writeback);
  }
}

void Pipe::Read(uint8_t* dest_buffer, int read_size, int* size_writeback) {
  if (closed_) {
    // assuming calling proc's page table is in cr3
    *size_writeback = 0;
  } else {
    GetFile()->Read(this, dest_buffer, read_size, size_writeback);
  }
}

void Pipe::Close() {
  GetFile()->Close(this);
  closed_ = true;
}

}  // namespace ipc

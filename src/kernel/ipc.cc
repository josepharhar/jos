#include "ipc.h"

namespace ipc {

File::File() {}
File::~File() {}

Pipe::Pipe(File* file, Mode mode) : file_(file), mode_(mode) {}
Pipe::~Pipe() {}

File* Pipe::GetFile() {
  return file_;
}
Mode Pipe::GetMode() {
  return mode_;
}

void Pipe::Write(const uint8_t* source_buffer, int write_size, int* size_writeback) {
  GetFile()->Write(this, source_buffer, write_size, size_writeback);
}

void Pipe::Read(uint8_t* dest_buffer, int read_size, int* size_writeback) {
  GetFile()->Read(this, dest_buffer, read_size, size_writeback);
}

}  // namespace ipc

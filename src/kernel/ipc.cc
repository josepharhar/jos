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

}  // namespace ipc

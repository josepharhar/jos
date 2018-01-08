#include "buffer_file.h"

BufferFile::BufferFile() {}
BufferFile::~BufferFile() {}

ipc::Pipe* BufferFile::Open(ipc::Mode mode) {
  BufferPipe* pipe = new BufferPipe(this, mode);
  pipes_.Add(pipe);
  return pipe;
}

void BufferFile::Close(ipc::Pipe* pipe) {
  pipes_.RemoveValue(pipe);
  delete pipe;
}

int BufferFile::GetNumPipes() {
  return pipes_.Size();
}

BufferPipe::BufferPipe(ipc::File* file, ipc::Mode mode)
    : ipc::Pipe(file, mode), buffer_(4096) {}
BufferPipe::~BufferPipe() {}

int BufferPipe::Write(const uint8_t* source_buffer, int write_size) {
  return (int)buffer_.Write(source_buffer, (uint64_t)write_size);
}

int BufferPipe::Read(uint8_t* dest_buffer, int read_size) {
  return (int)buffer_.Read(dest_buffer, (uint64_t)read_size);
}

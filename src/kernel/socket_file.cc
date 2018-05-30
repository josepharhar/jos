#include "socket_file.h"

SocketFile::SocketFile() {}

ipc::Pipe* SocketFile::Open(ipc::Mode mode) {
  SocketPipe* pipe = new SocketPipe(this, mode);
  pipes_.Add(pipe);
  return pipe;
}

int SocketFile::Close(ipc::Pipe* pipe) {
  pipes_.RemoveValue(pipe);
  delete pipe;
}

int SocketFile::GetNumPipes() {
  return pipes_.Size();
}

void SocketFile::Write(ipc::Pipe* pipe,
                       const uint8_t* source_buffer,
                       int write_size,
                       int* size_writeback) {
}

void SocketFile::Read(ipc::Pipe* pipe,
                      uint8_t* dest_buffer,
                      int read_size,
                      int* size_writeback) {}

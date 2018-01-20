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

int BufferFile::Write(ipc::Pipe* pipe,
                      const uint8_t* source_buffer,
                      int write_size) {
  uint64_t write_size_available = buffer_.WriteSizeAvailable();
  if (write_size_available) {
    int size_written = (int)buffer_.Write(source_buffer, (uint64_t)write_size);

    // unblock block processes that are trying to read.
    // TODO is this defined behavior in linux/unix/POSIX?
    //int size_read = 0;
    while (!read_request_queue_.IsEmpty() && buffer_.ReadSizeAvailable()) {
      // TODO CANT DO THIS BECAUSE THE PROCESS WHICH THIS READ REQUEST CAME FROM IS NOT IN MEMORY AND WE HAVE TO SWITCH PAGE TABLES TO IT TO ACCESS ITS MEMORY
      // man 7 pipe
      RdWrRequest read_request = read_request_queue_.Remove();
      Read(read_request.pipe, read_request.buffer, read_request.size);
      read_request_queue_.UnblockHead();
    }

    return size_written;
  }

  // block this process until there is space to write.
  RdWrRequest write_request;
  write_request.pipe = pipe;
  write_request.buffer = source_buffer;
  write_request.
}

int BufferFile::Read(ipc::Pipe* pipe, uint8_t* dest_buffer, int read_size) {
  return (int)buffer_.Read(dest_buffer, (uint64_t)read_size);
}

BufferPipe::BufferPipe(ipc::File* file, ipc::Mode mode)
    : ipc::Pipe(file, mode), buffer_(4096) {}
BufferPipe::~BufferPipe() {}

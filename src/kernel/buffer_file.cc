#include "buffer_file.h"

BufferFile::BufferFile() : buffer_(4096) {}
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
                      const uint8_t* source_physical_address,
                      int write_size) {
  uint64_t write_size_available = buffer_.WriteSizeAvailable();
  if (write_size_available) {
    int size_written = (int)buffer_.Write(source_buffer, (uint64_t)write_size);

    // unblock block processes that are trying to read.
    // TODO is this defined behavior in linux/unix/POSIX?
    // man 7 pipe
    //int size_read = 0;
    while (!read_request_queue_.IsEmpty() && buffer_.ReadSizeAvailable()) {
      // TODO CANT DO THIS BECAUSE THE PROCESS WHICH THIS READ REQUEST CAME FROM IS NOT IN MEMORY AND WE HAVE TO SWITCH PAGE TABLES TO IT TO ACCESS ITS MEMORY
      // TODO access the page table to figure out the physical address, and use the identity map to fulfill this request?
      RdWrRequest read_request = read_request_queue_.Remove();
      Read(read_request.pipe, read_request.physical_address, read_request.size);
      //read_request_queue_.UnblockHead();
      read_blocked_queue_.UnblockHead();
    }

    return size_written;
  }

  // block this process until there is space to write.
  RdWrRequest write_request;
  write_request.pipe = pipe;
  write_request.physical_address = (uint8_t*)source_buffer;
  write_request.size = write_size;
  
  // TODO
  return -1;
}

int BufferFile::Read(ipc::Pipe* pipe, uint8_t* dest_physical_address, int read_size) {
  return (int)buffer_.Read(dest_physical_address, (uint64_t)read_size);
}

BufferPipe::BufferPipe(ipc::File* file, ipc::Mode mode)
    : ipc::Pipe(file, mode) {}
BufferPipe::~BufferPipe() {}

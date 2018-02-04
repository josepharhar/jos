#include "buffer_file.h"

#include "kernel/page.h"
#include "printk.h"

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
                      const uint8_t* source_buffer,
                      int write_size) {
  uint64_t write_size_available = buffer_.WriteSizeAvailable();
  if (write_size_available) {
    int size_written = (int)buffer_.Write(source_buffer, (uint64_t)write_size);

    // unblock block processes that are trying to read.
    // TODO is this defined behavior in linux/unix/POSIX?
    // TODO based on manual linux testing, the LAST proc to block gets it.
    // man 7 pipe
    while (!read_request_queue_.IsEmpty() && buffer_.ReadSizeAvailable()) {
      // fulfill this read request.
      RdWrRequest read_request = read_request_queue_.Remove();

      int bytes_to_read = read_request.size;
      if (buffer_.ReadSizeAvailable() < bytes_to_read) {
        bytes_to_read = buffer_.ReadSizeAvailable();
      }

      Setcr3(read_request.proc->cr3);
      buffer_.Read(read_request.buffer, bytes_to_read);
      Setcr3(proc::GetCurrentProc()->cr3);

      printk("BufferFile::Write unblocking a proc\n");
      read_blocked_queue_.UnblockHead();
    }

    return size_written;
  }

  // block this process until there is space to write.
  RdWrRequest write_request;
  write_request.pipe = pipe;
  write_request.buffer = (uint8_t*)source_buffer;
  write_request.size = write_size;
  write_request.proc = proc::GetCurrentProc();
  write_request_queue_.Add(write_request);

  // this will make syscall yield
  write_blocked_queue_.BlockCurrentProcNoNesting();
  // TODO is this reachable?
  printk("BufferFile::Write this is supposed to be non-reachable!\n");
  return -1;
}

int BufferFile::Read(ipc::Pipe* pipe, uint8_t* dest_buffer, int read_size) {
  printk("BufferFile::Read dest_buffer: %p, read_size: %d\n", dest_buffer, read_size);
  uint64_t read_size_available = buffer_.ReadSizeAvailable();
  if (read_size_available) {
    int size_read = (int)buffer_.Read(dest_buffer, (uint64_t)read_size);

    // unblock procs that are trying to write
    while (!write_request_queue_.IsEmpty() && buffer_.WriteSizeAvailable()) {
      RdWrRequest write_request = write_request_queue_.Remove();

      int bytes_to_write = write_request.size;
      if (buffer_.WriteSizeAvailable() < bytes_to_write) {
        bytes_to_write = buffer_.WriteSizeAvailable();
      }

      Setcr3(write_request.proc->cr3);
      buffer_.Write(write_request.buffer, bytes_to_write);
      Setcr3(proc::GetCurrentProc()->cr3);

      write_blocked_queue_.UnblockHead();
    }

    return size_read;
  }

  // block this process until there is stuff to read.
  RdWrRequest read_request;
  read_request.pipe = pipe;
  read_request.buffer = dest_buffer;
  read_request.size = read_size;
  read_request.proc = proc::GetCurrentProc();
  read_request_queue_.Add(read_request);

  // this will make syscall yield
  read_blocked_queue_.BlockCurrentProcNoNesting();
  // TODO is this reachable?
  printk("BufferFile::Read this is supposed to be non-reachable!\n");
  return -1;
}

BufferPipe::BufferPipe(ipc::File* file, ipc::Mode mode)
    : ipc::Pipe(file, mode) {}
BufferPipe::~BufferPipe() {}

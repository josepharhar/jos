#include "socket_file.h"

#include "net.h"
#include "tcp.h"
#include "packets.h"

SocketFile::SocketFile(TcpAddr addr) : buffer_(8192) {
  handle_ = net::OpenTcpConnection(addr, GlobalIncomingPacketHandler, this,
                                   GlobalSocketClosedHandler, this);
}

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
  // TODO
}

void SocketFile::Read(ipc::Pipe* pipe,
                      uint8_t* dest_buffer,
                      int read_size,
                      int* size_writeback) {
  // read from buffer if there is anything available, else queue
  if (buffer_.ReadSizeAvailable()) {
    int amount_read = buffer_.Read(dest_buffer, read_size);
    *size_writeback = amount_read;
    return;
  }

  ReadRequest new_request;
  new_request.buffer_writeback = dest_buffer;
  new_request.buffer_length = (uint64_t)write_size;
  new_request.size_writeback = size_writeback;

  proc_blocked_queue_.BlockCurrentProcNoNesting();
  proc_context_queue_.Add(proc::GetCurrentProc());
  request_queue.Add(new_request);
}

// static
void SocketFile::GlobalSocketClosedHandler(void* arg) {
  ((SocketFile)arg)->SocketClosedHandler();
}

// static
void SocketFile::GlobalIncomingPacketHandler(void* packet,
                                             uint64_t length,
                                             void* arg) {
  ((SocketFile)arg)->IncomingPacketHandler(packet, length);
}

void SocketFile::SocketClosedHandler() {
  // TODO when the socket it closed, we have to return EOF (zero)
  //   to all read/write calls
}

void SocketFile::IncomingPacketHandler(void* packet, uint64_t length) {
  if (buffer_.WriteSizeAvailable() < length) {
    printk("SocketFile::IncomingPacketHandler not enough buffer space\n");
    return;
  }
  buffer_.Write(packet, length);
  kfree(packet);
  packet = 0;

  // now try and drain the buffer
  if (proc_context_queue_.Size()) {
    proc::ProcContext* proc = proc_context_queue_.Remove();
    ReadRequest request = request_queue_.Remove();
    proc_blocked_queue.UnblockHead();

    uint64_t read_length = request.buffer_length;
    if (buffer_.ReadSizeAvailable() < read_length) {
      read_length = buffer_.ReadSizeAvailable();
    }

    uint64_t old_cr3 = Getcr3();
    bool switch_tables = old_cr3 != proc->cr3;
    if (switch_tables) {
      Setcr3(proc->cr3);
    }
    buffer_.Read(request.buffer_writeback, read_length);
    *(request.size_writeback) = read_length;
    if (switch_tables) {
      Setcr3(old_cr3);
    }
  }
}

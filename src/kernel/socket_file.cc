#include "socket_file.h"

#include "net.h"
#include "tcp.h"
#include "packets.h"
#include "asm.h"
#include "printk.h"
#include "kmalloc.h"

SocketFile::SocketFile(TcpAddr addr)
    : buffer_(8192), connection_closed_(false) {
  handle_ = net::OpenTcpConnection(addr, GlobalIncomingPacketHandler, this,
                                   GlobalSocketClosedHandler, this);
}

ipc::Pipe* SocketFile::Open(ipc::Mode mode) {
  ipc::Pipe* pipe = new ipc::Pipe(this, mode);
  pipes_.Add(pipe);
  return pipe;
}

void SocketFile::Close(ipc::Pipe* pipe) {
  pipes_.RemoveValue(pipe);
}

int SocketFile::GetNumPipes() {
  return pipes_.Size();
}

void SocketFile::Write(ipc::Pipe* pipe,
                       const uint8_t* source_buffer,
                       int write_size,
                       int* size_writeback) {
  if (connection_closed_) {
    *size_writeback = 0;
    return;
  }

  // pass buffer directly to network driver
  net::SendTcpPacket((void*)source_buffer, write_size, handle_);
}

void SocketFile::Read(ipc::Pipe* pipe,
                      uint8_t* dest_buffer,
                      int read_size,
                      int* size_writeback) {
  // so when the connection is closed, there may still be some
  // buffered data left. if there is, and we read all of it,
  // then destroy everything. else this needs to stay alive
  // AND the pipes need to stay open.

  // read from buffer if there is anything available, else queue
  if (buffer_.ReadSizeAvailable()) {
    int amount_read = buffer_.Read(dest_buffer, read_size);
    *size_writeback = amount_read;
    if (!buffer_.ReadSizeAvailable() && connection_closed_) {
      CloseEverything();
    }
    return;
  }

  if (connection_closed_) {
    *size_writeback = 0;
    CloseEverything();
    return;
  }

  ReadRequest new_request;
  new_request.buffer_writeback = dest_buffer;
  new_request.buffer_length = (uint64_t)read_size;
  new_request.size_writeback = size_writeback;

  proc_blocked_queue_.BlockCurrentProcNoNesting();
  proc_context_queue_.Add(proc::GetCurrentProc());
  request_queue_.Add(new_request);
}

// static
void SocketFile::GlobalSocketClosedHandler(void* arg) {
  ((SocketFile*)arg)->SocketClosedHandler();
}

// static
void SocketFile::GlobalIncomingPacketHandler(void* packet,
                                             uint64_t length,
                                             void* arg) {
  ((SocketFile*)arg)->IncomingPacketHandler(packet, length);
}

void SocketFile::SocketClosedHandler() {
  connection_closed_ = true;
  // if there is still stuff left in the read buffer, stay alive.
  if (!buffer_.ReadSizeAvailable()) {
    CloseEverything();
  }
}

void SocketFile::IncomingPacketHandler(void* packet, uint64_t length) {
  if (buffer_.WriteSizeAvailable() < length) {
    printk("SocketFile::IncomingPacketHandler not enough buffer space\n");
    return;
  }
  buffer_.Write((const uint8_t*)packet, length);
  kfree(packet);
  packet = 0;

  // now try and drain the buffer
  if (proc_context_queue_.Size()) {
    proc::ProcContext* proc = proc_context_queue_.Remove();
    ReadRequest request = request_queue_.Remove();
    proc_blocked_queue_.UnblockHead();

    uint64_t read_length = request.buffer_length;
    if (buffer_.ReadSizeAvailable() < read_length) {
      read_length = buffer_.ReadSizeAvailable();
    }

    uint64_t old_cr3 = Getcr3();
    bool switch_tables = old_cr3 != proc->cr3;
    if (switch_tables) {
      Setcr3(proc->cr3);
    }
    buffer_.Read((uint8_t*)request.buffer_writeback, read_length);
    *(request.size_writeback) = read_length;
    if (switch_tables) {
      Setcr3(old_cr3);
    }
  }
}

void SocketFile::CloseEverything() {
  // mark all pipes as closed.
  // Pipe::Close will modify our array by calling ClosePipe(), so make a copy
  stdj::Array<ipc::Pipe*> pipes_copy = pipes_;
  for (int i = 0; i < pipes_copy.Size(); i++) {
    pipes_copy.Get(i)->Close();
  }

  // unblock and return zero on all blocked processes
  while (proc_context_queue_.Size()) {
    proc::ProcContext* proc = proc_context_queue_.Remove();
    proc_blocked_queue_.UnblockHead();
    ReadRequest request = request_queue_.Remove();

    uint64_t old_cr3 = Getcr3();
    bool switch_tables = old_cr3 != proc->cr3;
    if (switch_tables) {
      Setcr3(proc->cr3);
    }
    *(request.size_writeback) = 0;
    if (switch_tables) {
      Setcr3(old_cr3);
    }
  }

  // TODO delete this?
}

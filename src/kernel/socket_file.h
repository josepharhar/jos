#ifndef KERNEL_SOCKET_FILE_H_
#define KERNEL_SOCKET_FILE_H_

#include "ipc.h"
#include "proc.h"
#include "jarray.h"
#include "jbuffer.h"

class SocketFile : public ipc::File {
 public:
  SocketFile();

  ipc::Pipe* Open(ipc::Mode mode) override;
  int Close(ipc::Pipe* pipe) override;
  int GetNumPipes() override;

  void Write(ipc::Pipe* pipe,
             const uint8_t* source_buffer,
             int write_size,
             int* size_writeback) override;
  void Read(ipc::Pipe* pipe,
            uint8_t* dest_buffer,
            int read_size,
            int* size_writeback) override;

 private:
  struct ReadRequest {
    void* buffer_writeback;
    uint64_t buffer_length;
    int status_writeback;
  };

  stdj::Array<ipc::Pipe*> pipes_;
  TcpHandle handle_;
  //stdj::Queue<std::pair<void*, uint64_t>> incoming_packet_queue_;
  stdj::Buffer<uint8_t> buffer_;

  proc::BlockedQueue proc_blocked_queue_;
  stdj::Queue<proc::ProcContext*> proc_context_queue_;
  stdj::Queue<ReadRequest> request_queue_;

  static void GlobalSocketClosedHandler(void* arg);
  static void GlobalIncomingPacketHandler(void* packet,
                                          uint64_t length,
                                          void* arg);

  void SocketClosedHandler();
  void IncomingPacketHandler(void* packet, uint64_t length);
};

class SocketPipe : public ipc::Pipe {
 public:
  SocketPipe(ipc::File* file, ipc::Mode mode);
};

#endif  // KERNEL_SOCKET_FILE_H_

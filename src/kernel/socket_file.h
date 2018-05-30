#ifndef KERNEL_SOCKET_FILE_H_
#define KERNEL_SOCKET_FILE_H_

#include "ipc.h"
#include "proc.h"
#include "jarray.h"
#include "jbuffer.h"
#include "tcp.h"
#include "packets.h"

class SocketFile : public ipc::File {
 public:
  SocketFile(TcpAddr addr);

  ipc::Pipe* Open(ipc::Mode mode) override;
  void Close(ipc::Pipe* pipe) override;
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
    int* size_writeback;
  };

  stdj::Array<ipc::Pipe*> pipes_;
  net::TcpHandle handle_;
  stdj::Buffer<uint8_t> buffer_;

  proc::BlockedQueue proc_blocked_queue_;
  stdj::Queue<proc::ProcContext*> proc_context_queue_;
  stdj::Queue<ReadRequest> request_queue_;

  bool connection_closed_;

  static void GlobalSocketClosedHandler(void* arg);
  static void GlobalIncomingPacketHandler(void* packet,
                                          uint64_t length,
                                          void* arg);

  void SocketClosedHandler();
  void IncomingPacketHandler(void* packet, uint64_t length);

  void CloseEverything();
};

#endif  // KERNEL_SOCKET_FILE_H_

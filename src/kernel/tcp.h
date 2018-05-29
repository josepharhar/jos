#ifndef KERNEL_TCP_H_
#define KERNEL_TCP_H_

#include "net.h"
#include "packets.h"

namespace net {

struct TcpHandle {
  TcpHandle();
  TcpHandle(uint16_t new_local_port, TcpAddr new_remote_addr);
  static const TcpHandle INVALID;

  uint16_t local_port;
  TcpAddr remote_addr;

  uint64_t ToNumber() const;
  bool operator==(const TcpHandle& other);
  bool operator!=(const TcpHandle& other);
};
bool operator<(const TcpHandle& left, const TcpHandle& right);

typedef int TcpHandle;
typedef void (*TcpPacketHandler)(void*, uint64_t, void*);
typedef void (*TcpConnectionClosedHandler)(void*);

TcpHandle OpenTcpConnection(IpAddr dest,
                            uint16_t port,
                            TcpPacketHandler handler,
                            void* handler_arg,
                            TcpConnectionClosedHandler closed_handler,
                            void* closed_handler_arg);

void SendTcpPacket(void* packet, uint64_t length, TcpHandle handle);

void HandleTcpPacket(Ethernet* ethernet, uint64_t length);

}  // namespace net

#endif  // KERNEL_TCP_H_

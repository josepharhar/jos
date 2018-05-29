#include "tcp.h"

namespace net {

TcpHandle OpenTcpConnection(IpAddr dest,
                            uint16_t port,
                            TcpPacketHandler handler,
                            void* handler_arg,
                            TcpConnectionClosedHandler closed_handler,
                            void* closed_handler_arg) {
  static TcpHandle next_handle = 1;
  TcpHandle new_handle = next_handle++;

  return new_handle;
}

void SendTcpPacket(void* packet, uint64_t length, TcpHandle handle) {

}

}  // namespace net

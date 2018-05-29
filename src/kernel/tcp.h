#ifndef KERNEL_TCP_H_
#define KERNEL_TCP_H_

#include "net.h"

namespace net {

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

}  // namespace net

#endif  // KERNEL_TCP_H_

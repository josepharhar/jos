#ifndef KERNEL_ICMP_H_
#define KERNEL_ICMP_H_

#include "packets.h"

namespace net {

typedef void (*PingCallback)(void*);
void Ping(IpAddr ip_address, PingCallback callback, void* callback_param);

void HandleIcmpPacket(Ethernet* ethernet, uint64_t length);

}  // namespace net

#endif  // KERNEL_ICMP_H_

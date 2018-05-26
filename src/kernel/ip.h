#ifndef KERNEL_IP_H_
#define KERNEL_IP_H_

#include "packets.h"

namespace net {

void HandleIpPacket(Ethernet* ethernet, uint64_t length);
void SendIpPacket(void* packet,
                  uint64_t length,
                  IpAddr dest,
                  uint8_t ip_protocol);

}  // namespace net

#endif  // KERNEL_IP_H_

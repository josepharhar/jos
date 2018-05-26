#ifndef KERNEL_NET_H_
#define KERNEL_NET_H_

#include "stdint.h"
#include "packets.h"

namespace net {

void Init();

typedef void (*PacketReceivedHandler)(uint8_t*, uint64_t);
void SendPacket(void* packet, uint64_t length);
void SetPacketReceivedHandler(PacketReceivedHandler handler);

IpAddr GetMyIp();
Mac GetMyMac();

}

#endif  // KERNEL_NET_H_

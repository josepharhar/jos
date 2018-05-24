#ifndef KERNEL_NET_H_
#define KERNEL_NET_H_

#include "stdint.h"

namespace net {

void Init();

typedef void (*PacketReceivedHandler)(uint8_t*, uint64_t);
void SendPacket();
void SetPacketReceivedHandler(PacketReceivedHandler handler);

}

#endif  // KERNEL_NET_H_

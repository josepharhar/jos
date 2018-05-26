#include "ethernet.h"

#include "net.h"
#include "kmalloc.h"
#include "packets.h"

namespace net {

void SendEthernetPacket(void* packet,
                        uint64_t packet_length,
                        Mac dest,
                        uint16_t ethernet_type) {
  uint64_t ethernet_length = sizeof(Ethernet) + packet_length;
  Ethernet* ethernet = (Ethernet*)kmalloc(ethernet_length);
  ethernet->SetDest(dest);
  ethernet->SetSrc(GetMyMac());
  ethernet->SetType(ethernet_type);
  memcpy(ethernet + 1, packet, packet_length);
  SendPacket(ethernet, ethernet_length);
  kfree(ethernet);
}

}  // namsepace net

#ifndef ETHERNET_H_
#define ETHERNET_H_

#include "packets.h"

namespace net {

void SendEthernetPacket(void* packet,
                        uint64_t packet_length,
                        Mac dest,
                        uint16_t ethernet_type);
}

#endif  // ETHERNET_H_

#include "ip.h"

#include "icmp.h"

namespace net {

void HandleIpPacket(Ethernet* ethernet, uint64_t ip_length) {
  IP* ip = (IP*)(ethernet + 1);
  printk("received ipv4 packet\n");

  if (ip->GetDestAddr() != GetMyIp()) {
    // packet was not addressed to us.
    return;
  }

  switch (ip->protocol) {
    case IP_PROTOCOL_ICMP:
      HandleIcmpPacket((ICMP*)(ip + 1), ip_length - sizeof(IP));
      break;

    case IP_PROTOCOL_TCP:
      printk("received tcp packet\n");
      break;

    case IP_PROTOCOL_UDP:
      printk("received udp packet\n");
      break;
  }
}

}  // namespace net

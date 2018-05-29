#include "ip.h"

#include "icmp.h"
#include "printk.h"
#include "net.h"
#include "kmalloc.h"
#include "ethernet.h"
#include "arp.h"
#include "tcp.h"

namespace net {

void HandleIpPacket(Ethernet* ethernet, uint64_t length) {
  IP* ip = (IP*)(ethernet + 1);
  printk("received ipv4 packet\n");

  if (ip->GetDest() != GetMyIp()) {
    // packet was not addressed to us.
    return;
  }

  switch (ip->protocol) {
    case IP_PROTOCOL_ICMP:
      HandleIcmpPacket(ethernet, length);
      break;

    case IP_PROTOCOL_TCP:
      HandleTcpPacket(ethernet, length);
      break;

    case IP_PROTOCOL_UDP:
      printk("received udp packet\n");
      break;
  }
}

struct SendIpContext {
  IP* ip;
  uint64_t ip_length;
};
static void GotMacCallback(Mac dest, void* arg) {
  SendIpContext* context = (SendIpContext*)arg;
  SendEthernetPacket(context->ip, context->ip_length, dest, ETHERTYPE_IP);
  kfree(context->ip);
  delete context;
}
void SendIpPacket(void* packet,
                  uint64_t length,
                  IpAddr dest,
                  uint8_t ip_protocol) {
  uint64_t ip_length = sizeof(IP) + length;
  IP* ip = (IP*)kmalloc(ip_length);
  ip->version = IP_VERSION_IPV4;
  ip->length = (sizeof(IP) * 8) / 32;
  ip->tos = 0;  // TODO what should this be
  ip->SetTotalLength(ip_length);
  static uint16_t next_id = 1;
  ip->SetId(next_id++);
  ip->flags = 0;
  ip->fragment_offset = 0;
  ip->time_to_live = 128;  // TODO is this too big?
  ip->protocol = ip_protocol;
  ip->checksum = 0;
  ip->SetSource(GetMyIp());
  ip->SetDest(dest);
  memcpy(ip + 1, packet, length);
  //ip->checksum = in_cksum(ip, ip_length);
  ip->checksum = in_cksum(ip, sizeof(IP));

  // TODO do subnet logic here to figure out if
  // the ip dest it outside of our LAN and if it should be sent
  // to the gateway ip or just the straight up ip
  SendIpContext* context = new SendIpContext();
  context->ip = ip;
  context->ip_length = ip_length;
  ArpGetIp(GetGatewayIp(), GotMacCallback, context);
}

}  // namespace net

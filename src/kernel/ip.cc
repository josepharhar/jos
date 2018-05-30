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
  if (length < sizeof(Ethernet) + sizeof(IP)) {
    // packet is invalid, it says ip but doesnt have enough room for ip header
    printk("HandleIpPacket packet says its ip but isnt long enough to be\n");
    return;
  }
  IP* ip = (IP*)(ethernet + 1);

  uint64_t ip_total_length = ip->GetTotalLength();
  if (ip_total_length < sizeof(IP)) {
    printk("HandleIpPacket packet says its shorter than ip\n");
    // invalid, must at least have header
    return;
  }
  if (length - sizeof(Ethernet) < ip_total_length) {
    // invalid, packet says its longer than it actually is
    printk("HandleIpPacket packet says its longer than it is\n");
    return;
  }
  if (length - sizeof(Ethernet) > ip_total_length) {
    // this happens normally i guess, from ethernet padding or something?
    /*printk("HandleIpPacket packet says its %d, actually is %d\n",
           ip_total_length + sizeof(Ethernet), length);*/
    length = ip_total_length + sizeof(Ethernet);
  }

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
  // ip->checksum = in_cksum(ip, ip_length);
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

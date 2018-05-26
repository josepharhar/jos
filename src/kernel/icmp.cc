#include "icmp.h"

#include "net.h"
#include "arp.h"

namespace net {

// TODO add ping data here and check it
// when we get a reply to identify individual
// request/response chains
struct PingRequest {
  IpAddr target;
  PingCallback callback;
  void* callback_arg;
};
static stdj::Array<PingRequest>* ping_requests = new stdj::Array<PingState>();

void Ping(IpAddr ip_address, PingCallback callback, void* callback_param) {
  PingRequest ping_request;
  ping_request.target = ip_address;
  ping_request.callback = callback;
  ping_request.callback_param = callback_param;
  ping_requests->Add(ping_request);

  ICMP* icmp = (ICMP*)kmalloc(sizeof(ICMP));
  icmp->type = ICMP_TYPE_PING_REQUEST;
  icmp->code = ICMP_CODE_PING;
  memset(icmp->rest, 0, 4);
  icmp->checksum = in_cksum(icmp, sizeof(ICMP));
  SendIpPacket(icmp, sizeof(ICMP), ip_address, IP_PROTOCOL_ICMP);
  kfree(icmp);
}

void HandleIcmpPacket(Ethernet* ethernet, uint64_t length) {
  IP* ip = (IP*)(ethernet + 1);
  ICMP* icmp = (ICMP*)(ip + 1);

  switch (icmp->type) {
    case ICMP_TYPE_PING_REQUEST:
      // TODO make a ping reply?
      break;

    case ICMP_TYPE_PING_REPLY: {
      IpAddr source = ip->GetSource();
      for (int i = ping_requests->Size() - 1; i >= 0; i--) {
        PingRequest request = ping_requests->Get(i);
        if (request.target == source) {
          request.callback(request.callback_arg);
          ping_requests->RemoveAt(i);
        }
      }
      break;
    }

    default:
      printk("unrecognized icmp type: %d\n", icmp->type);
      break;
  }
}

}  // namespace net

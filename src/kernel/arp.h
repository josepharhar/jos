#ifndef ARP_H_
#define ARP_H_

#include "packets.h"

namespace net {

typedef void (*ArpGotIpCallback)(IpAddr, void*);
// returns true if there was a cache miss and an arp request will be made.
bool ArpGetIp(IpAddr target, ArpGotIpCallback callback, void* callback_arg);

}  // namespace net

#endif  // ARP_H_

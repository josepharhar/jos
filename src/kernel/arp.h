#ifndef ARP_H_
#define ARP_H_

#include "packets.h"

namespace net {

void HandleArp(ARP* arp, uint64_t arp_size);

typedef void (*ArpGotMacCallback)(Mac, void*);
// returns true if there was a cache miss and an arp request will be made.
bool ArpGetIp(IpAddr target, ArpGotMacCallback callback, void* callback_arg);

}  // namespace net

#endif  // ARP_H_

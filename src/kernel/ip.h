#ifndef KERNEL_IP_H_
#define KERNEL_IP_H_

namespace net {

void HandleIpPacket(Ethernet* ethernet, uint64_t ip_length);

}  // namespace net

#endif  // KERNEL_IP_H_

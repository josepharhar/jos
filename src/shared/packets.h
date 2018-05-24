#ifndef PACKETS_H_
#define PACKETS_H_

#include "string.h"

uint16_t ntohs(uint16_t network_short);
uint32_t ntohl(uint32_t network_long);
uint16_t htons(uint16_t host_short);
uint32_t htonl(uint32_t host_long);
unsigned short in_cksum(void* addr, int len);

// https://www.tcpdump.org/pcap.html

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IPV6 0x86DD

#define IP_PROTOCOL_ICMP 1
#define IP_PROTOCOL_TCP 6
#define IP_PROTOCOL_UDP 17

#define ARP_HARDWARE_TYPE_ETHERNET 1
#define ARP_PROTOCOL_IP 0x0800
#define ARP_OPCODE_REQUEST 1
#define ARP_OPCODE_REPLY 2
#define ARP_HARDWARE_SIZE 6
#define ARP_PROTOCOL_SIZE 4

#define ICMP_PING_REQUEST 8
#define ICMP_PING_REPLY 0

class MAC {
 public:
  MAC() {}
  MAC(const uint8_t* new_addr) { memcpy(addr, new_addr, 6); }
  MAC(uint8_t one,
      uint8_t two,
      uint8_t three,
      uint8_t four,
      uint8_t five,
      uint8_t six) {
    addr[0] = one;
    addr[1] = two;
    addr[2] = three;
    addr[3] = four;
    addr[4] = five;
    addr[5] = six;
  }

  uint8_t addr[6];

  uint64_t ToNumber() const {
    uint64_t number = 0;
    memcpy(&number, addr, 6);
    return number;
  }

  bool operator==(const MAC& other) {
    for (int i = 0; i < 6; i++) {
      if (addr[i] != other.addr[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const MAC& other) { return !operator==(other); }
  friend bool operator<(const MAC& left, const MAC& right) {
    return left.ToNumber() < right.ToNumber();
  }
} __attribute__((packed));

class IPAddr {
 public:
  IPAddr() {}
  IPAddr(const uint8_t* new_addr) { memcpy(addr, new_addr, 4); }
  IPAddr(uint8_t one, uint8_t two, uint8_t three, uint8_t four) {
    addr[0] = one;
    addr[1] = two;
    addr[2] = three;
    addr[3] = four;
  }

  uint8_t addr[4];

  uint64_t ToNumber() const {
    uint64_t number = 0;
    memcpy(&number, addr, 4);
    return number;
  }
  bool operator==(const IPAddr& other) {
    for (int i = 0; i < 4; i++) {
      if (addr[i] != other.addr[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const IPAddr& other) { return !operator==(other); }
  friend bool operator<(const IPAddr& left, const IPAddr& right) {
    return left.ToNumber() < right.ToNumber();
  }
} __attribute__((packed));

class Ethernet {
 public:
  uint8_t mac_dest[6];
  uint8_t mac_src[6];

 private:
  uint16_t type;

 public:
  uint16_t GetType() { return ntohs(type); }
  void SetType(uint16_t new_type) { type = htons(new_type); }
  MAC GetDest() { return MAC(mac_dest); }
  MAC GetSrc() { return MAC(mac_src); }
} __attribute__((packed));

static const uint8_t MAC_BCAST_ARRAY[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const MAC MAC_BCAST(MAC_BCAST_ARRAY);

class IP {
 public:
  uint8_t version : 4;
  uint8_t length : 4;                     // sizeof this struct / 32
  uint8_t differentiated_services_field;  // aka TOS
 private:
  uint16_t total_length;  // entire packet - sizeof ethernet header
  uint16_t identification;

 public:
  uint8_t flags;
  uint8_t fragment_offset;
  uint8_t time_to_live;
  uint8_t protocol;  // TCP = 6
 private:
  uint16_t checksum;

 public:
  uint8_t source[4];
  uint8_t destination[4];

 public:
  uint16_t GetTotalLength() { return ntohs(total_length); }
  uint16_t GetId() { return ntohs(identification); }
  uint16_t GetChecksum() { return ntohs(checksum); }
} __attribute__((packed));

class ARP {
 private:
  uint16_t hardware_type;  // 1 = ethernet
  uint16_t protocol;       // 0x0800 = ipv4
 public:
  uint8_t hardware_size;
  uint8_t protocol_size;

 private:
  uint16_t opcode;  // 1 = request
 public:
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];

 public:
  void SetHardwareType(uint16_t new_hardware_type) {
    hardware_type = htons(new_hardware_type);
  }
  void SetProtocol(uint16_t new_protocol) { protocol = htons(new_protocol); }
  void SetOpcode(uint16_t new_opcode) { opcode = htons(new_opcode); }
  uint16_t GetOpcode() { return ntohs(opcode); }
  void SetSourceMac(MAC mac) { memcpy(sender_mac, mac.addr, 6); }
  void SetSourceIp(IPAddr ip) { memcpy(sender_ip, ip.addr, 4); }
  void SetTargetMac(MAC mac) { memcpy(target_mac, mac.addr, 6); }
  void SetTargetIp(IPAddr ip) { memcpy(target_ip, ip.addr, 6); }
  MAC GetSourceMac() { return MAC(sender_mac); }
  MAC GetTargetMac() { return MAC(target_mac); }
  IPAddr GetSourceIp() { return IPAddr(sender_ip); }
  IPAddr GetTargetIp() { return IPAddr(target_ip); }
} __attribute__((packed));

class ICMP {
 public:
  uint8_t type;
  uint8_t code;

 private:
  uint16_t checksum;

 public:
  uint8_t rest[4];

 public:
  uint16_t GetChecksum() { return ntohs(checksum); }
} __attribute__((packed));

#endif  // PACKETS_H_

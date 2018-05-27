#ifndef PACKETS_H_
#define PACKETS_H_

#include "string.h"
#include "stdlib.h"
#include "jarray.h"
#include "jstring.h"

// https://www.tcpdump.org/pcap.html

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IPV6 0x86DD

#define IP_PROTOCOL_ICMP 1
#define IP_PROTOCOL_TCP 6
#define IP_PROTOCOL_UDP 17
#define IP_VERSION_IPV4 4

#define ARP_HARDWARE_TYPE_ETHERNET 1
#define ARP_PROTOCOL_IP 0x0800
#define ARP_OPCODE_REQUEST 1
#define ARP_OPCODE_REPLY 2
#define ARP_HARDWARE_SIZE 6
#define ARP_PROTOCOL_SIZE 4

#define ICMP_TYPE_PING_REQUEST 8
#define ICMP_TYPE_PING_REPLY 0
#define ICMP_CODE_PING 0

uint16_t ntohs(uint16_t network_short);
uint32_t ntohl(uint32_t network_long);
uint16_t htons(uint16_t host_short);
uint32_t htonl(uint32_t host_long);
unsigned short in_cksum(void* addr, int len);

class Mac {
 public:
  // Mac() : Mac(0, 0, 0, 0, 0, 0) {}
  constexpr Mac() : addr{0, 0, 0, 0, 0, 0} {}
  Mac(const uint8_t* new_addr) { memcpy(addr, new_addr, 6); }
  Mac(uint8_t one,
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

  bool operator==(const Mac& other) {
    for (int i = 0; i < 6; i++) {
      if (addr[i] != other.addr[i]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const Mac& other) { return !operator==(other); }
  friend bool operator<(const Mac& left, const Mac& right) {
    return left.ToNumber() < right.ToNumber();
  }
} __attribute__((packed));

class IpAddr {
 public:
  IpAddr();
  IpAddr(const uint8_t* new_addr);
  IpAddr(uint8_t one, uint8_t two, uint8_t three, uint8_t four);
  static IpAddr FromString(char* string);

  uint8_t addr[4];

  stdj::string ToString();
  uint64_t ToNumber() const;
  bool operator==(const IpAddr& other);
  bool operator!=(const IpAddr& other);
  friend bool operator<(const IpAddr& left, const IpAddr& right) {
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
  Mac GetDest() { return Mac(mac_dest); }
  Mac GetSrc() { return Mac(mac_src); }
  void SetDest(Mac mac) { memcpy(mac_dest, mac.addr, 6); }
  void SetSrc(Mac mac) { memcpy(mac_src, mac.addr, 6); }
} __attribute__((packed));

class IP {
 public:
  uint8_t version : 4;
  uint8_t length : 4;  // sizeof this struct / 32
  // uint8_t differentiated_services_field;  // aka TOS
  uint8_t tos;  // aka TOS
 private:
  uint16_t total_length;  // entire packet - sizeof ethernet header
  uint16_t id;

 public:
  uint8_t flags;
  uint8_t fragment_offset;
  uint8_t time_to_live;
  uint8_t protocol;  // TCP = 6
  uint16_t checksum;
  uint8_t source[4];
  uint8_t dest[4];

 public:
  uint16_t GetTotalLength() { return ntohs(total_length); }
  uint16_t GetId() { return ntohs(id); }
  uint16_t GetChecksum() { return ntohs(checksum); }
  void SetTotalLength(uint16_t new_total_length) {
    total_length = htons(new_total_length);
  }
  void SetId(uint16_t new_id) { id = ntohs(new_id); }
  void SetDest(IpAddr addr) { memcpy(dest, addr.addr, 4); }
  void SetSource(IpAddr addr) { memcpy(source, addr.addr, 4); }
  IpAddr GetDest() { return IpAddr(dest); }
  IpAddr GetSource() { return IpAddr(source); }
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
  void SetSourceMac(Mac mac) { memcpy(sender_mac, mac.addr, 6); }
  void SetSourceIp(IpAddr ip) { memcpy(sender_ip, ip.addr, 4); }
  void SetTargetMac(Mac mac) { memcpy(target_mac, mac.addr, 6); }
  void SetTargetIp(IpAddr ip) { memcpy(target_ip, ip.addr, 6); }
  Mac GetSourceMac() { return Mac(sender_mac); }
  Mac GetTargetMac() { return Mac(target_mac); }
  IpAddr GetSourceIp() { return IpAddr(sender_ip); }
  IpAddr GetTargetIp() { return IpAddr(target_ip); }
} __attribute__((packed));

class ICMP {
 public:
  uint8_t type;
  uint8_t code;
  uint16_t checksum;

 public:
  uint8_t rest[4];

 public:
  uint16_t GetChecksum() { return ntohs(checksum); }
} __attribute__((packed));

void PrintMac(Mac mac);
void PrintIp(IpAddr ip);

#endif  // PACKETS_H_

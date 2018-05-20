#ifndef PACKETS_H_
#define PACKETS_H_

#include "string.h"

uint16_t ntohs(uint16_t network_short) {
  return ((network_short << 8) & 0xFF00) | ((network_short >> 8) & 0x00FF);
}

uint32_t ntohl(uint32_t network_long) {
  return ((network_long << (8 + 8 + 8)) & 0xFF000000) |
         ((network_long << 8) & 0x00FF0000) |
         ((network_long >> 8) & 0x0000FF00) |
         ((network_long >> (8 + 8 + 8)) & 0x000000FF);
}

uint16_t htons(uint16_t host_short) {
  return ntohs(host_short);
}

uint32_t htonl(uint32_t host_long) {
  return ntohl(host_long);
}

// https://www.tcpdump.org/pcap.html

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IPV6 0x86DD

#define PROTOCOL_ICMP 1
#define PROTOCOL_TCP 6
#define PROTOCOL_UDP 17

#define OPCODE_REQUEST 1
#define OPCODE_REPLY 2

#define ICMP_PING_REQUEST 8
#define ICMP_PING_REPLY 0

class MAC {
 public:
  MAC() {}
  MAC(const uint8_t* new_addr) { memcpy(addr, new_addr, 6); }

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

class Ethernet {
 public:
  uint8_t mac_dest[6];
  uint8_t mac_src[6];

 private:
  uint16_t type;

 public:
  uint16_t GetType() { return ntohs(type); }
  void SetType(uint16_t new_type) { type = htons(new_type); }
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
} __attribute__((packed));

#endif  // PACKETS_H_

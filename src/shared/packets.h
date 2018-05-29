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
  Mac();
  Mac(const uint8_t* new_addr);
  Mac(uint8_t one,
      uint8_t two,
      uint8_t three,
      uint8_t four,
      uint8_t five,
      uint8_t six);
  static Mac FromString(const char* string);
  static const Mac INVALID;

  stdj::string ToString();
  uint64_t ToNumber() const;
  bool operator==(const Mac& other);
  bool operator!=(const Mac& other);

  // TODO if this is 6 then it causes segfaults
  uint8_t addr[16];
} __attribute__((packed));
bool operator<(const Mac& left, const Mac& right);

class IpAddr {
 public:
  IpAddr();
  IpAddr(const uint8_t* new_addr);
  IpAddr(uint8_t one, uint8_t two, uint8_t three, uint8_t four);
  static IpAddr FromString(const char* string);
  static const IpAddr INVALID;

  stdj::string ToString();
  uint64_t ToNumber() const;
  bool operator==(const IpAddr& other);
  bool operator!=(const IpAddr& other);

  uint8_t addr[16];
} __attribute__((packed));
bool operator<(const IpAddr& left, const IpAddr& right);

class TcpAddr {
 public:
  TcpAddr();
  TcpAddr(IpAddr ip_addr, uint16_t port);
  static TcpAddr FromString(const char* string);
  static const TcpAddr INVALID;
  static const uint16_t INVALID_PORT;

  stdj::string ToString();
  uint64_t ToNumber() const;
  bool operator==(const TcpAddr& other);
  bool operator!=(const TcpAddr& other);

  uint16_t port;
  IpAddr ip_addr;
};
bool operator<(const TcpAddr& left, const TcpAddr& right);

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
  uint8_t length : 4;  // (sizeof this struct * 8) / 32, number of 32b words
  uint8_t version : 4;
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
  void* GetNextHeader() {
    // TODO this should take the length and return null for security
    return ((uint8_t*)this) + (length * 4);
  }
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

class TCPFlags {
 public:
  TCPFlags() { memset(this, 0, sizeof(TCPFlags)); }
  TCPFlags(uint8_t flags) { memcpy(this, &flags, sizeof(TCPFlags)); }

  uint8_t fin : 1;
  uint8_t syn : 1;
  uint8_t rst : 1;
  uint8_t psh : 1;
  uint8_t ack : 1;
  uint8_t urg : 1;
  uint8_t ece : 1;
  uint8_t cwr : 1;

  uint8_t GetValue() const {
    uint8_t value;
    memcpy(&value, this, sizeof(TCPFlags));
    return value;
  }

  bool operator==(const TCPFlags& other) {
    return GetValue() == other.GetValue();
  }
  bool operator!=(const TCPFlags& other) { return !operator==(other); }
} __attribute__((packed));

class TCP {
 private:
  uint16_t src_port;
  uint16_t dest_port;
  uint32_t seq;
  uint32_t ack_number;

 public:
  uint8_t ns : 1;
  uint8_t reserved : 3;
  uint8_t data_offset : 4;  // length of header / 4

  /*uint8_t fin : 1;
  uint8_t syn : 1;
  uint8_t rst : 1;
  uint8_t psh : 1;
  uint8_t ack : 1;
  uint8_t urg : 1;
  uint8_t ece : 1;
  uint8_t cwr : 1;*/
  uint8_t flags;

 private:
  uint16_t window_size;

 public:
  uint16_t checksum;

 private:
  uint16_t urgent_pointer;

 public:
  uint16_t GetSrcPort() { return ntohs(src_port); }
  void SetSrcPort(uint16_t new_src_port) { src_port = htons(new_src_port); }
  uint16_t GetDestPort() { return ntohs(dest_port); }
  void SetDestPort(uint16_t new_dest_port) { dest_port = htons(new_dest_port); }
  uint32_t GetSeq() { return ntohl(seq); }
  void SetSeq(uint32_t new_seq) { seq = htonl(new_seq); }
  uint32_t GetAckNumber() { return ntohl(ack_number); }
  void SetAckNumber(uint32_t new_ack_number) {
    ack_number = htonl(new_ack_number);
  }
  uint16_t GetWindowSize() { return ntohs(window_size); }
  void SetWindowSize(uint16_t new_window_size) {
    window_size = htons(new_window_size);
  }
  TCPFlags* GetFlags() { return (TCPFlags*)&flags; }
  int GetHeaderLength() { return data_offset * 4; }
} __attribute__((packed));
static_assert(sizeof(TCP) == 20, "wrong TCP size");

class TCPPseudoHeader {
 public:
  uint8_t src_ip[4];
  uint8_t dest_ip[4];
  uint8_t reserved;
  uint8_t protocol;

 private:
  uint16_t tcp_length;

 public:
  void SetTcpLength(uint16_t new_tcp_length) {
    tcp_length = htons(new_tcp_length);
  }
  void SetSrc(IpAddr addr) { memcpy(src_ip, addr.addr, 4); }
  void SetDest(IpAddr addr) { memcpy(dest_ip, addr.addr, 4); }
} __attribute__((packed));

void PrintMac(Mac mac);
void PrintIp(IpAddr ip);

#endif  // PACKETS_H_

#include "packets.h"

#include "stdio.h"

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

unsigned short in_cksum(void* addr, int len) {
  int sum = 0;
  unsigned short answer = 0;
  unsigned short* w = (unsigned short*)addr;
  int nleft = len;

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1) {
    *(unsigned char*)(&answer) = *(unsigned char*)w;
    sum += answer;
  }

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
  sum += (sum >> 16);                 /* add carry */
  answer = ~sum;                      /* truncate to 16 bits */
  return (answer);
}

void PrintMac(Mac mac) {
  printf("%02X:%02X:%02X:%02X:%02X:%02X", mac.addr[0], mac.addr[1], mac.addr[2],
         mac.addr[3], mac.addr[4], mac.addr[5]);
}

void PrintIp(IpAddr ip) {
  printf("%d.%d.%d.%d", ip.addr[0], ip.addr[1], ip.addr[2], ip.addr[3]);
}

IpAddr::IpAddr() {}
IpAddr::IpAddr(const uint8_t* new_addr) {
  memcpy(addr, new_addr, 4);
}
IpAddr::IpAddr(uint8_t one, uint8_t two, uint8_t three, uint8_t four) {
  addr[0] = one;
  addr[1] = two;
  addr[2] = three;
  addr[3] = four;
}

// static
IpAddr IpAddr::FromString(const char* string) {
  stdj::string jstring(string);
  stdj::Array<stdj::string> splits = jstring.Split(".");
  if (splits.Size() != 4) {
    return INVALID;
  }
  IpAddr addr(0, 0, 0, 0);
  stdj::string str = splits.Get(0);
  addr.addr[0] = atoi(str.c_str());
  str = splits.Get(1);
  addr.addr[1] = atoi(str.c_str());
  str = splits.Get(2);
  addr.addr[2] = atoi(str.c_str());
  str = splits.Get(3);
  addr.addr[3] = atoi(str.c_str());
  return addr;
}

stdj::string IpAddr::ToString() {
  stdj::string output = "";
  for (int i = 0; i < 4; i++) {
    if (i != 0) {
      output = output + stdj::string(".");
    }
    output = output + stdj::string::ParseInt(addr[i]);
  }
  return output;
}

uint64_t IpAddr::ToNumber() const {
  uint64_t number = 0;
  memcpy(&number, addr, 4);
  return number;
}

bool IpAddr::operator==(const IpAddr& other) {
  for (int i = 0; i < 4; i++) {
    if (addr[i] != other.addr[i]) {
      return false;
    }
  }
  return true;
}

bool IpAddr::operator!=(const IpAddr& other) {
  return !operator==(other);
}

stdj::string Mac::ToString() {
  stdj::string output = "";
  for (int i = 0; i < 6; i++) {
    if (i != 0) {
      output = output + stdj::string(":");
    }
    output = output + stdj::string::ParseInt(addr[i], 16, 2);
  }
  return output;
}

// Mac

Mac::Mac() {}

Mac::Mac(uint8_t one,
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

Mac::Mac(const uint8_t* new_addr) {
  memcpy(addr, new_addr, 6);
}

const Mac Mac::INVALID = Mac(0x42, 0x42, 0x42, 0x42, 0x42, 0x42);
const IpAddr IpAddr::INVALID = IpAddr(192, 0, 2, 42);

uint64_t Mac::ToNumber() const {
  uint64_t number = 0;
  memcpy(&number, addr, 6);
  return number;
}

bool Mac::operator==(const Mac& other) {
  for (int i = 0; i < 6; i++) {
    if (addr[i] != other.addr[i]) {
      return false;
    }
  }
  return true;
}
bool Mac::operator!=(const Mac& other) {
  return !operator==(other);
}

bool operator<(const Mac& left, const Mac& right) {
  return left.ToNumber() < right.ToNumber();
}

bool operator<(const IpAddr& left, const IpAddr& right) {
  return left.ToNumber() < right.ToNumber();
}

// static
Mac Mac::FromString(const char* string) {
  stdj::string jstring(string);
  stdj::Array<stdj::string> splits = jstring.Split(":");
  if (splits.Size() != 6) {
    return INVALID;
  }
  Mac mac_addr(0, 0, 0, 0, 0, 0);
  mac_addr.addr[0] = splits.Get(0).ToInt(16);
  mac_addr.addr[1] = splits.Get(1).ToInt(16);
  mac_addr.addr[2] = splits.Get(2).ToInt(16);
  mac_addr.addr[3] = splits.Get(3).ToInt(16);
  mac_addr.addr[4] = splits.Get(4).ToInt(16);
  mac_addr.addr[5] = splits.Get(5).ToInt(16);
  return mac_addr;
}

TcpAddr::TcpAddr() {}

TcpAddr::TcpAddr(IpAddr new_ip_addr, uint16_t new_port)
    : ip_addr(new_ip_addr), port(new_port) {}

// static
TcpAddr TcpAddr::FromString(const char* string) {
  stdj::string jstring(string);
  stdj::Array<stdj::string> port_split = jstring.Split(":");
  if (port_split.Size() != 2) {
    return INVALID;
  }
  stdj::string ip_string = port_split.Get(0);
  IpAddr ip_addr = IpAddr::FromString(ip_string.c_str());
  if (ip_addr == IpAddr::INVALID) {
    return INVALID;
  }

  uint16_t port = (uint16_t)port_split.Get(1).ToInt();
  return TcpAddr(ip_addr, port);
}

stdj::string TcpAddr::ToString() {
  return ip_addr.ToString() + stdj::string(":") + stdj::string::ParseInt(port);
}

uint64_t TcpAddr::ToNumber() const {
  return ip_addr.ToNumber() + (((uint64_t)port) << 32);
}

bool TcpAddr::operator==(const TcpAddr& other) {
  return ip_addr == other.ip_addr && port == other.port;
}

bool TcpAddr::operator!=(const TcpAddr& other) {
  return !operator==(other);
}

bool operator<(const TcpAddr& left, const TcpAddr& right) {
  return left.ToNumber() < right.ToNumber();
}

const TcpAddr TcpAddr::INVALID = TcpAddr(IpAddr::INVALID, 42);

#undef TEST
#include "shared/packets.h"

#include <assert.h>
#include <stdio.h>

void TestEndian() {
  uint16_t example_short = 0x4880;
  uint16_t inverted_short = 0x8048;
  uint32_t example_long = 0x12345678;
  uint32_t inverted_long = 0x78563412;
  assert(inverted_short == ntohs(example_short));
  assert(inverted_short == htons(example_short));
  assert(inverted_long == ntohl(example_long));
  assert(inverted_long == htonl(example_long));

  assert(IpAddr(192, 168, 0, 1).ToNumber() ==
         IpAddr::FromString("192.168.0.1").ToNumber());
}

void TestIpAddrParsing() {
  IpAddr addr(192, 168, 0, 1);
  assert(addr.ToString() == stdj::string("192.168.0.1"));
  assert(addr == IpAddr::FromString("192.168.0.1"));
}

void TestMacAddrParsing() {
  Mac addr(0xff, 0x00, 0x01, 0x20, 0x23, 0xa3);
  assert(addr.ToString() == stdj::string("FF:00:01:20:23:A3"));
  assert(addr == Mac::FromString("ff:00:01:20:23:a3"));
  assert(addr == Mac::FromString("fF:0:1:20:23:a3"));
}

void TestTcpAddrParsing() {
  TcpAddr addr(IpAddr(192, 168, 0, 1), 80);
  assert(addr.ToString() == stdj::string("192.168.0.1:80"));
  assert(addr == TcpAddr::FromString("192.168.0.1:80"));
}

int main(int argc, char** argv) {
  TestEndian();
  TestIpAddrParsing();
  TestMacAddrParsing();
  TestTcpAddrParsing();
  return 0;
}

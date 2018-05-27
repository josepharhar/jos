#undef TEST
#include "shared/packets.h"

#include <assert.h>

int main(int argc, char** argv) {
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

  return 0;
}

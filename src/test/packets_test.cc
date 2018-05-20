#include "test.h"

#include <arpa/inet.h>

namespace jos_packets {
#include "shared/packets.h"
}

int main(int argc, char** argv) {
  uint16_t example_short = 0x4880;
  uint32_t example_long = 0x12345678;
  ASSERT_EQ(ntohs(example_short), jos_packets::ntohs(example_short));
  ASSERT_EQ(htons(example_short), jos_packets::htons(example_short));
  ASSERT_EQ(htonl(example_long), jos_packets::htonl(example_long));
  ASSERT_EQ(ntohl(example_long), jos_packets::ntohl(example_long));
  return 0;
}

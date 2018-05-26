#include "test.h"

#include <arpa/inet.h>

namespace jos {
#include "shared/packets.h"
}

int main(int argc, char** argv) {
  uint16_t example_short = 0x4880;
  uint32_t example_long = 0x12345678;
  ASSERT_EQ(ntohs(example_short), jos::ntohs(example_short));
  ASSERT_EQ(htons(example_short), jos::htons(example_short));
  ASSERT_EQ(htonl(example_long), jos::htonl(example_long));
  ASSERT_EQ(ntohl(example_long), jos::ntohl(example_long));

  ASSERT_EQ(jos::IpAddr(192, 168, 0, 1),
            jos::IpAddr::FromString("192.168.0.1"));

  return 0;
}

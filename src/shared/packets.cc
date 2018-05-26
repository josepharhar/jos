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

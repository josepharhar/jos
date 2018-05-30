#include "socket.h"
#include "stdio.h"
#include "stdlib.h"
#include "jstring.h"
#include "jarray.h"
#include "packets.h"
#include "unistd.h"

#define BUFFER_SIZE 2048

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("usage: wget 1.2.3.4:80 /filepath\n");
    exit(1);
  }
  char* tcp_addr_string = argv[1];
  char* filepath = argv[2];

  TcpAddr tcp_addr = TcpAddr::FromString(tcp_addr_string);
  if (tcp_addr == TcpAddr::INVALID) {
    printf("invalid tcp address: %s\n", tcp_addr_string);
    exit(1);
  }

  int socket_fd = socket(tcp_addr_string);
  if (socket_fd < 0) {
    printf("wget socket() returned invalid fd: %d\n", socket_fd);
    exit(1);
  }
  printf("wget got socket fd: %d\n", socket_fd);

  stdj::string http_request =
      stdj::string("GET ") + stdj::string(filepath) + stdj::string("\n\n");
  write(socket_fd, http_request.c_str(), http_request.Size());
  printf("wget wrote http request\n");

  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  int bytes_read = read(socket_fd, buffer, BUFFER_SIZE);
  while (bytes_read) {
    printf("wget read() %d bytes\n", bytes_read);
    bytes_read = read(socket_fd, buffer, BUFFER_SIZE);
  }

  printf("wget finished reading. buffer:\n%s\n", buffer);

  exit(0);
}

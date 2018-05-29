#include "tcp.h"

#include "jmap.h"

namespace net {

static uint32_t tcp_rand() {
  static uint32_t next_rand = 0x4880;
  return next_rand++;
}

class TcpConnection {
 public:
  enum State {
    kUninitialized = 0,
    kClosed = 1,
    kSynSent = 2,
    kEstablished = 3,
  };

  TcpConnection(TcpHandle handle) : handle_(handle), state_(kUninitialized) {}

  void Start() {
    SetState(kClosed);
    my_seq_ = tcp_rand() % UINT32_MAX + 1;

 private:
  TcpHandle handle_;
  State state_;
  uint32_t my_seq_;
  uint32_t other_seq_;
  uint32_t init_my_seq_;
  uint32_t init_other_seq_;
};

// static stdj::DefaultMap<TcpHandle, TcpState> handle_to_state;
static stdj::Array<TcpConnection> tcp_connections;

static TcpHandle GenerateHandle(TcpAddr remote_addr) {
  TcpHandle new_handle;
  new_handle.remote_addr = remote_addr;
  for (uint16_t i = 49152; i < 65535; i++) {
    new_handle.local_port = i;
    if (!handle_to_state.ContainsKey(new_handle)) {
      return new_handle;
    }
  }
  return TcpHandle::INVALID;
}

TcpHandle OpenTcpConnection(TcpAddr remote_addr,
                            TcpPacketHandler handler,
                            void* handler_arg,
                            TcpConnectionClosedHandler closed_handler,
                            void* closed_handler_arg) {
  TcpHandle new_handle = GenerateHandle(remote_addr);

  TcpState new_state;
  new_state.remote_addr = addr;

  handle_to_state.Put(new_handle, new_state);

  return new_handle;
}

void SendTcpPacket(void* packet, uint64_t length, TcpHandle handle) {}

void HandleTcpPacket(Ethernet* ethernet, uint64_t length) {
  IP* ip = (IP*)(ethernet + 1);
  TCP* tcp = (TCP*)ip->GetNextHeader();  // TODO this is unsecure
}

// TcpHandle implementation

TcpHandle::TcpHandle() {}

TcpHandle::TcpHandle(uint16_t new_local_port, TcpAddr new_remote_addr)
    : local_port(new_local_port), remote_addr(new_remote_addr) {}

const TcpHandle TcpHandle::INVALID =
    TcpHandle(TcpAddr::INVALID_PORT, TcpAddr::INVALID);

bool TcpHandle::operator==(const TcpHandle& other) {
  return remote_addr == other.remote_addr && local_port == other.local_port;
}

bool TcpHandle::operator!=(const TcpHandle& other) {
  return operator==(other);
}

uint64_t TcpHandle::ToNumber() {
  return remote_addr.ToNumber() + (((uint64_t)local_port) << 48);
}

bool operator<(const TcpHandle& left, const TcpHandle& right) {
  return left.ToNumber() < right.ToNumber();
}

}  // namespace net

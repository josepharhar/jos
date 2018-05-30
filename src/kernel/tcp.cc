#include "tcp.h"

#include "jmap.h"
#include "jarray.h"
#include "jpair.h"
#include "kmalloc.h"
#include "printk.h"
#include "ip.h"

namespace net {

class TcpConnection;
static stdj::Array<TcpConnection*> tcp_connections;

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

  TcpConnection(TcpHandle handle,
                TcpPacketHandler packet_handler,
                void* packet_handler_arg,
                TcpConnectionClosedHandler closed_handler,
                void* closed_handler_arg)
      : handle_(handle),
        state_(kUninitialized),
        packet_handler_(packet_handler),
        packet_handler_arg_(packet_handler_arg),
        closed_handler_(closed_handler),
        closed_handler_arg_(closed_handler_arg) {}

  TcpHandle GetHandle() { return handle_; }

  void Start() {
    SetState(kClosed);
    my_seq_ = tcp_rand() % UINT32_MAX + 1;

    TCPFlags send_flags;
    send_flags.syn = 1;
    Send(0, 0, send_flags.GetValue());
    SetState(kSynSent);

    // invisible syn byte
    my_seq_++;
  }

  int Receive(TCP* tcp, uint64_t length) {
    uint64_t payload_length = length - tcp->data_offset * 4;

    printk("received tcp. state: %d\n", state_);
    printk("  fin: %d, syn: %d, rst: %d, psh: %d\n", tcp->GetFlags()->fin,
           tcp->GetFlags()->syn, tcp->GetFlags()->rst, tcp->GetFlags()->psh);
    printk("  ack: %d, urg: %d, ece: %d, cwr: %d\n", tcp->GetFlags()->ack,
           tcp->GetFlags()->urg, tcp->GetFlags()->ece, tcp->GetFlags()->cwr);

    switch (state_) {
      case kUninitialized:
        DCHECK(false);
        break;

      case kClosed:
        printk("  state is kClosed, ignoring received packet\n");
        break;

      case kSynSent: {
        // packet coming back should be SYN-ACK
        TCPFlags expected_flags;
        expected_flags.syn = 1;
        expected_flags.ack = 1;
        if (*(tcp->GetFlags()) != expected_flags) {
          printk("  packet coming back should have been synack.\n");
          Cancel();
          break;
        }
        init_other_seq_ = tcp->GetSeq();
        other_seq_ = tcp->GetSeq();

        other_seq_++;  // TODO when exactly should this be incremented?
        // the ack number we send back is supposed to be
        // the last seq we fully received + 1.

        // send an ack
        printk("  got synack. sending ack\n");
        TCPFlags ack_flags;
        ack_flags.ack = 1;
        Send(0, 0, ack_flags.GetValue());

        SetState(kEstablished);

        // send all buffered packets
        for (unsigned i = 0; i < buffered_packets_to_send_.Size(); i++) {
          auto buffered_packet = buffered_packets_to_send_.Get(i);
          Send(buffered_packet.first, buffered_packet.second);
          kfree((void*)buffered_packet.first);
        }
        buffered_packets_to_send_ = stdj::Array<stdj::pair<const void*, int>>();

        break;
      }

      case kEstablished:
        if (tcp->GetSeq() != other_seq_) {
          printk("  BAD tcp->GetSeq(): %u\n", tcp->GetSeq());
          printk("         other_seq_: %u\n", other_seq_);
          printk("    init_other_seq_: %u\n", init_other_seq_);
          break;
        }
        if (payload_length) {
          packet_handler_(tcp + 1, payload_length, packet_handler_arg_);
        }
        if (!payload_length) {
          // printk("  no payload, should other_seq_ be incremented??\n");
        }
        other_seq_ += payload_length;

        if (tcp->GetFlags()->fin) {
          // TODO only send fin back when we are done sending data n stuff.
          printk("  received fin, sending fin back.\n");
          TCPFlags fin_flags;
          fin_flags.fin = 1;
          fin_flags.ack = 1;
          other_seq_++;  // TODO delet this
          Send(0, 0, fin_flags.GetValue());

          return 1;
        }
        break;
    }
  }

  void Cancel() {
    switch (state_) {
      case kUninitialized:
      case kClosed:
        break;
      case kSynSent:
      case kEstablished: {
        printk("  sending reset\n");
        TCPFlags reset_flags;
        reset_flags.rst = 1;
        Send(0, 0, reset_flags.GetValue());
        break;
      }
    }
    SetState(kClosed);
    closed_handler_(closed_handler_arg_);
    tcp_connections.RemoveValue(this);
    delete this;
  }

  void Send(const void* buffer, int buffer_length) {
    switch (state_) {
      case kUninitialized:
      case kClosed:
      case kSynSent: {
        void* buffer_copy = kmalloc(buffer_length);
        memcpy(buffer_copy, buffer, buffer_length);
        buffered_packets_to_send_.Add(
            stdj::pair<const void*, int>(buffer_copy, buffer_length));
        return;
      }

      case kEstablished:
        break;
    }

    TCPFlags flags;
    flags.ack = 1;
    flags.psh = 1;
    Send(buffer, buffer_length, flags.GetValue());
  }

 private:
  TcpHandle handle_;
  State state_;
  uint32_t my_seq_;
  uint32_t other_seq_;
  uint32_t init_my_seq_;
  uint32_t init_other_seq_;
  stdj::Array<stdj::pair<const void*, int>> buffered_packets_to_send_;
  TcpPacketHandler packet_handler_;
  void* packet_handler_arg_;
  TcpConnectionClosedHandler closed_handler_;
  void* closed_handler_arg_;

  void Send(const void* buffer, uint64_t buffer_length, uint8_t flags) {
    uint16_t tcp_length = sizeof(TCP) + buffer_length;
    uint16_t tcp_pseudo_length = sizeof(TCPPseudoHeader) + tcp_length;

    TCPPseudoHeader* pseudo_header =
        (TCPPseudoHeader*)kcalloc(tcp_pseudo_length);

    pseudo_header->SetSrc(GetMyIp());
    pseudo_header->SetDest(handle_.remote_addr.ip_addr);
    pseudo_header->reserved = 0;
    pseudo_header->protocol = IP_PROTOCOL_TCP;
    pseudo_header->SetTcpLength(tcp_length);

    TCP* tcp = (TCP*)(pseudo_header + 1);
    tcp->SetSrcPort(handle_.local_port);
    tcp->SetDestPort(handle_.remote_addr.port);
    tcp->SetSeq(my_seq_);
    my_seq_ += buffer_length;
    tcp->SetAckNumber(other_seq_);
    tcp->data_offset = sizeof(TCP) / 4;
    *(tcp->GetFlags()) = flags;
    tcp->SetWindowSize(29200);

    memcpy(tcp + 1, buffer, buffer_length);
    tcp->checksum = 0;
    tcp->checksum = in_cksum(pseudo_header, tcp_pseudo_length);

    SendIpPacket(tcp, tcp_length, handle_.remote_addr.ip_addr, IP_PROTOCOL_TCP);
    kfree(pseudo_header);
  }

  void SetState(State new_state) {
    switch (new_state) {
      case kUninitialized:
        DCHECK(false);
        break;
      case kClosed:
        break;
      case kSynSent:
        if (state_ != kClosed) {
          printk("state_: %d, new_state: %d\n", state_, new_state);
        }
        DCHECK(state_ == kClosed);
        break;
      case kEstablished:
        DCHECK(false);
        break;
    }
    state_ = new_state;
  }
};

static TcpConnection* GetConnection(TcpHandle handle) {
  for (int i = 0; i < tcp_connections.Size(); i++) {
    TcpConnection* connection = tcp_connections.Get(i);
    if (connection->GetHandle() == handle) {
      return connection;
    }
  }
  return 0;
}

static TcpHandle GenerateHandle(TcpAddr remote_addr) {
  TcpHandle new_handle;
  new_handle.remote_addr = remote_addr;
  for (uint16_t i = 49152; i < 65535; i++) {
    new_handle.local_port = i;
    if (!GetConnection(new_handle)) {
      return new_handle;
    }
  }
  return TcpHandle::INVALID;
}

TcpHandle OpenTcpConnection(TcpAddr remote_addr,
                            TcpPacketHandler packet_handler,
                            void* packet_handler_arg,
                            TcpConnectionClosedHandler closed_handler,
                            void* closed_handler_arg) {
  TcpHandle new_handle = GenerateHandle(remote_addr);
  tcp_connections.Add(new TcpConnection(new_handle, packet_handler,
                                        packet_handler_arg, closed_handler,
                                        closed_handler_arg));
  return new_handle;
}

void SendTcpPacket(void* packet, uint64_t length, TcpHandle handle) {
  TcpConnection* connection = GetConnection(handle);
  if (connection) {
    connection->Send(packet, length);
  }
  // TODO signal an error if there is no connection?
}

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

uint64_t TcpHandle::ToNumber() const {
  return remote_addr.ToNumber() + (((uint64_t)local_port) << 48);
}

bool operator<(const TcpHandle& left, const TcpHandle& right) {
  return left.ToNumber() < right.ToNumber();
}

}  // namespace net

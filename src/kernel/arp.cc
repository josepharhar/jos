#include "arp.h"

#include "jmap.h"
#include "jarray.h"
#include "printk.h"

namespace net {

typedef stdj::DefaultMap<IpAddr, Mac> ArpTable;
static ArpTable* arp_table = 0;

struct IpRequest {
  ArpGotMacCallback callback;
  void* callback_arg;
};

typedef stdj::DefaultMap<IpAddr, stdj::Array<IpRequest>> RequestTable;
static RequestTable* ip_to_requests = 0;

static void AddRequest(IpAddr addr, IpRequest request) {
  if (!ip_to_requests) {
    ip_to_requests = new RequestTable();
  }
  if (!ip_to_requests->ContainsKey(addr)) {
    ip_to_requests->Get(addr) = stdj::Array<IpRequest>();
  }
  // TODO can i use copy elision stuff to make this better?
  stdj::Array<IpRequest> new_request_array = ip_to_requests->Get(addr);
  new_request_array.Add(request);
  ip_to_requests->Set(addr, new_request_array);
}
static stdj::Array<IpRequest> GetRequests(IpAddr addr) {
  if (!ip_to_requests || !ip_to_requests->ContainsKey(addr)) {
    return stdj::Array<IpRequest>();
  }
  return ip_to_requests->Get(addr);
}
static void ClearRequests(IpAddr addr) {
  if (!ip_to_requests || !ip_to_requests->ContainsKey(addr)) {
    return;
  }
  ip_to_requests->Remove(addr);
}

bool ArpGetIp(IpAddr target, ArpGotMacCallback callback, void* callback_arg) {
  if (!arp_table) {
    arp_table = new ArpTable();
  }

  if (arp_table->ContainsKey(target)) {
    callback(arp_table->Get(target), callback_arg);
    return false;
  }

  // target wasn't found, make an arp request for it
  IpRequest new_request;
  new_request.callback = callback;
  new_request.callback_arg = callback_arg;
  AddRequest(target, new_request);
  return true;
}

void HandleArp(ARP* arp, uint64_t arp_size) {
  if (!arp_table) {
    arp_table = new ArpTable();
  }

  if (arp->GetOpcode() == ARP_OPCODE_REPLY) {
    printk("received arp reply from: ");
    PrintMac(arp->GetSourceMac());
    printk(" ");
    PrintIp(arp->GetSourceIp());
    printk("\n");

    IpAddr new_ip = arp->GetSourceIp();
    Mac new_mac = arp->GetSourceMac();
    arp_table->Set(new_ip, new_mac);

    stdj::Array<IpRequest> requests = GetRequests(new_ip);
    for (int i = 0; i < requests.Size(); i++) {
      IpRequest request = requests.Get(i);
      request.callback(new_mac, request.callback_arg);
    }
    ClearRequests(new_ip);

  } else if (arp->GetOpcode() == ARP_OPCODE_REQUEST) {
    printk("received arp request for: ");
    PrintMac(arp->GetTargetMac());
    printk("\n");
    // TODO make arp response
  }

}

}  // namespace net

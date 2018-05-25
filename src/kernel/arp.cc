#include "arp.h"

#include "jmap.h"

namespace net {

typedef stdj::DefaultMap<IpAddr, Mac> ArpTable;
static ArpTable* arp_table = 0;

struct IpRequest {
  ArpGotIpCallback callback;
  void* callback_arg;
};

static stdj::DefaultMap<IpAddr, stdj::Array<IpRequest> > ip_to_requests;
//void AddRequest(

bool ArpGetIp(IpAddr target, ArpGotIpCallback callback, void* callback_arg) {
  if (!arp_table) {
    arp_table = new ArpTable();
  }

  if (arp_table->ContainsKey(target)) {
    callback(arp_table->Get(target), callback_arg);
    return false;
  }

  // target wasn't found, make an arp request for it
}


arp_table = new stdj::DefaultMap<IpAddr, Mac>();

}  // namespace net

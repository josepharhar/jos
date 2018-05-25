#include "net.h"

#include "pci.h"
#include "e1000.h"
#include "packets.h"
#include "printk.h"
#include "kmalloc.h"
#include "irq.h"
#include "jmap.h"
#include "arp.h"

namespace net {

static E1000* driver = 0;
static Mac my_mac;
static IpAddr my_ip;
static Mac gateway_mac;
static IpAddr gateway_ip;

// static stdj::Array<PacketReceivedHandler> handlers;

static void PrintMac(Mac mac) {
  printk("%02X:%02X:%02X:%02X:%02X:%02X", mac.addr[0], mac.addr[1],
         mac.addr[2], mac.addr[3], mac.addr[4], mac.addr[5]);
}
static void PrintIp(IpAddr ip) {
  printk("%d.%d.%d.%d", ip.addr[0], ip.addr[1], ip.addr[2], ip.addr[3]);
}

static void HandleARP(Ethernet* ethernet, uint64_t length) {
  if (length < sizeof(Ethernet) + sizeof(ARP)) {
    printk("packet length is less than ethernet + arp: %d\n", length);
    return;
  }

  ARP* arp = (ARP*)(ethernet + 1);
  if (arp->GetOpcode() == ARP_OPCODE_REPLY) {
    printk("received arp reply from: ");
    PrintMac(arp->GetSourceMac());
    printk(" ");
    PrintIp(arp->GetSourceIp());
    printk("\n");
  } else if (arp->GetOpcode() == ARP_OPCODE_REQUEST) {
    printk("received arp request for: ");
    PrintMac(arp->GetTargetMac());
    printk("\n");
  }
}

static void HandlePacketReceived(uint8_t* packet, uint64_t length) {
  if (length < sizeof(Ethernet)) {
    printk("packet length is less than ethernet length: %d\n", length);
    return;
  }
  Ethernet* ethernet = (Ethernet*)packet;
  if (ethernet->GetDest() != my_mac) {
    // this packet wasn't addressed to us, throw it away.
    printk("packet wasnt sent to our mac: ");
    PrintMac(ethernet->GetDest());
    printk("\n");
    return;
  }

  switch (ethernet->GetType()) {
    case ETHERTYPE_IP:
      printk("received ipv4 packet\n");
      break;

    case ETHERTYPE_ARP:
      HandleARP(ethernet, length);
      break;

    case ETHERTYPE_IPV6:
      printk("received ipv6 packet\n");
      break;

    default:
      printk("unrecognized incoming ethertype: 0x%04X\n", ethernet->GetType());
      break;
  }
}

void SendPacket(uint8_t* packet, uint64_t length) {
  driver->sendPacket(packet, length);
}

void SetPacketReceivedHandler() {}

static void SendArp(IpAddr target) {
  int size = sizeof(Ethernet) + sizeof(ARP);
  if (size < 64) {
    // TODO min ethernet packet size?
    size = 64;
  }

  Ethernet* ethernet = (Ethernet*)kcalloc(size);
  memcpy(ethernet->mac_src, driver->getMacAddress(), 6);
  memset(ethernet->mac_dest, 0xFF, 6);
  ethernet->SetType(ETHERTYPE_ARP);

  ARP* arp = (ARP*)(ethernet + 1);
  arp->SetHardwareType(ARP_HARDWARE_TYPE_ETHERNET);
  arp->SetProtocol(ARP_PROTOCOL_IP);
  arp->hardware_size = ARP_HARDWARE_SIZE;
  arp->protocol_size = ARP_PROTOCOL_SIZE;
  arp->SetOpcode(ARP_OPCODE_REQUEST);
  memcpy(arp->sender_mac, driver->getMacAddress(), 6);
  arp->SetSourceIp(my_ip);
  memset(arp->target_mac, 0, 6);
  arp->SetTargetIp(target);

  driver->sendPacket(ethernet, size);
  kfree(ethernet);
}

static int StartE1000(pci::DeviceInfo device) {
  uint16_t command =
      pci::ReadConfig16(device.bus, device.device, 0, OFFSET_COMMAND);
  command = command | COMMAND_BUS_MASTER;
  command = command & ~COMMAND_PIO;
  command = command | COMMAND_MMIO;
  command = command & ~COMMAND_SERR;
  command = command & ~COMMAND_INTERRUPT_DISABLE;
  // command = command | COMMAND_INTERRUPT_DISABLE;
  command = command | COMMAND_WRITE_AND_INVALIDATE;
  command = command | COMMAND_SPECIAL_CYCLES;
  pci::WriteConfig16(device.bus, device.device, 0, OFFSET_COMMAND, command);

  uint8_t interrupt_line =
      pci::ReadConfig(device.bus, device.device, 0, OFFSET_INTERRUPT_LINE);
  uint64_t interrupt_number = (((uint64_t)interrupt_line) & 0xFF) + PIC1_OFFSET;
  pci::ReadConfig(device.bus, device.device, 0, OFFSET_INTERRUPT_LINE) +
      PIC1_OFFSET;
  uint32_t bar0 = pci::ReadConfig(device.bus, device.device, 0, OFFSET_BAR0);
  uint32_t bar1 = pci::ReadConfig(device.bus, device.device, 0, OFFSET_BAR1);
  driver = new E1000(interrupt_number, bar0, HandlePacketReceived);
  if (!driver->start()) {
    printk("driver->start() failed\n");
    return 1;
  }
  printk("driver->start() succeeded\n");
  my_mac = Mac(driver->getMacAddress());
  return 0;
}

static pci::DeviceInfo GetFirstE1000() {
  stdj::Array<pci::DeviceInfo> devices = pci::GetDevices();
  for (int i = 0; i < devices.Size(); i++) {
    pci::DeviceInfo device = devices.Get(i);
    if (device.vendor_id == VENDOR_INTEL && device.device_id == DEVICE_E1000) {
      return device;
    }
  }
  pci::DeviceInfo bad_device;
  bad_device.bus = 0xFFFF;
  return bad_device;
}

IpAddr GetMyIp() {
  return my_ip;
}

Mac GetMyMac() {
  return my_mac;
}

void Init() {
  driver = 0;
  my_mac = Mac(0, 0, 0, 0, 0, 0);
  gateway_mac = Mac(0, 0, 0, 0, 0, 0);
  // https://wiki.qemu.org/images/9/93/Slirp_concept.png
  my_ip = IpAddr(10, 0, 2, 15);
  gateway_ip = IpAddr(10, 0, 2, 2);

  pci::DeviceInfo device = GetFirstE1000();
  if (device.bus == 0xFFFF) {
    printk("failed to find an e1000 device\n");
    return;
  }
  printk("found an e1000 in bus %d, device %d\n", device.bus, device.device);
  StartE1000(device);
}

}  // namespace net

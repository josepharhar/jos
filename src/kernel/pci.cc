#include "pci.h"

#include "asm.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

struct Config {
  uint32_t enable : 1;
  uint32_t reserved : 7;
  uint32_t bus_number : 8;
  uint32_t device_number : 5;
  uint32_t function_number : 3;
  uint32_t register_number : 6;
  uint32_t zero : 2;
} __attribute__((packed));

uint16_t pciConfigReadWord(uint8_t bus,
                           uint8_t slot,
                           uint8_t func,
                           uint8_t offset) {
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;

  /* create configuration address as per Figure 1 */
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  sysOutLong(0xCF8, address);
  /* read in the data */
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  return (uint16_t)((sysInLong(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot) {
  uint16_t vendor, device;
  /* try and read the first configuration register. Since there are no */
  /* vendors that == 0xFFFF, it must be a non-existent device. */
  vendor = pciConfigReadWord(bus, slot, 0, 0);
  if (vendor != 0xFFFF) {
    device = pciConfigReadWord(bus, slot, 0, 2);
    // TODO
    //...
  }
  return vendor;
}

void checkDevice(uint8_t bus, uint8_t device) {
  uint8_t function = 0;

  vendorID = getVendorID(bus, device, function);
  if (vendorID == 0xFFFF)
    return;  // Device doesn't exist
  checkFunction(bus, device, function);
  headerType = getHeaderType(bus, device, function);
  if ((headerType & 0x80) != 0) {
    /* It is a multi-function device, so check remaining functions */
    for (function = 1; function < 8; function++) {
      if (getVendorID(bus, device, function) != 0xFFFF) {
        checkFunction(bus, device, function);
      }
    }
  }
}

void checkFunction(uint8_t bus, uint8_t device, uint8_t function) {}

void InitPci() {}

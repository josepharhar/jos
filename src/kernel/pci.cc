#include "pci.h"

#include "asm.h"
#include "printk.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

/*struct Config {
  uint32_t enable : 1;
  uint32_t reserved : 7;
  uint32_t bus_number : 8;
  uint32_t device_number : 5;
  uint32_t function_number : 3;
  uint32_t register_number : 6;
  uint32_t zero : 2;
} __attribute__((packed));*/

uint16_t pciConfigReadWord(uint8_t bus,
                           uint8_t device,
                           uint8_t func,
                           uint8_t offset) {
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t ldevice = (uint32_t)device;
  uint32_t lfunc = (uint32_t)func;

  /* create configuration address as per Figure 1 */
  address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outl(0xCF8, address);
  /* read in the data */
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t device) {
  uint16_t vendor_id, device_id;
  /* try and read the first configuration register. Since there are no */
  /* vendors that == 0xFFFF, it must be a non-existent device. */
  vendor_id = pciConfigReadWord(bus, device, 0, 0);
  if (vendor_id != 0xFFFF) {
    device_id = pciConfigReadWord(bus, device, 0, 2);
    printk("bus: 0x%X, device: 0x%X, vendor_id: 0x%X, device_id: 0x%X\n", bus,
           device, vendor_id, device_id);
  }
  return vendor_id;
}

/*void checkFunction(uint8_t bus, uint8_t device, uint8_t function) {
  printk("checkFunction bus: %d, device: %d, function: %d\n", bus, device,
         function);
}
void checkDevice(uint8_t bus, uint8_t device) {
  uint8_t function = 0;

  uint16_t vendorID = pciCheckVendor(bus, device, function);
  if (vendorID == 0xFFFF) {
    return;  // Device doesn't exist
  }
  checkFunction(bus, device, function);
  headerType = getHeaderType(bus, device, function);
  if ((headerType & 0x80) != 0) {
    // It is a multi-function device, so check remaining functions
    for (function = 1; function < 8; function++) {
      if (pciCheckVendor(bus, device, function) != 0xFFFF) {
        checkFunction(bus, device, function);
      }
    }
  }
}*/

void checkAllBuses() {
  uint16_t bus;
  uint8_t device;

  for (bus = 0; bus < 256; bus++) {
    for (device = 0; device < 32; device++) {
      // checkDevice(bus, device);
      pciCheckVendor((uint8_t)bus, device);
    }
  }
}

void InitPci() {
  printk("calling checkAllBuses()\n");
  checkAllBuses();
}

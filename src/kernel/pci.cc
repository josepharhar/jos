#include "pci.h"

#include "asm.h"
#include "printk.h"
#include "jarray.h"
#include "packets.h"
#include "kmalloc.h"
#include "irq.h"
#include "frame.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

namespace pci {

// TODO why do i have to reverse the order of the fields?
struct ConfigCommand {
  uint32_t register_number : 8;  // bottom two bits must be zero, 32 aligned
  uint32_t function_number : 3;
  uint32_t device_number : 5;
  uint32_t bus_number : 8;
  uint32_t reserved : 7;  // used in pci-express?
  uint32_t enable : 1;    // determines when accesses to CONFIG_DATA should be
                          // translated to configuration cycles
} __attribute__((packed));
static_assert(sizeof(ConfigCommand) == sizeof(uint32_t),
              "bad ConfigCommand size");

uint32_t ReadConfig(uint8_t bus,
                    uint8_t device,
                    uint8_t function,
                    uint8_t offset) {
  uint32_t address = 0;
  ConfigCommand* cmd = (ConfigCommand*)&address;
  cmd->enable = 1;
  cmd->bus_number = bus;
  cmd->device_number = device;
  cmd->function_number = function;
  cmd->register_number = offset & 0xFC;

  outl(0xCF8, address);
  return inl(0xCFC);
}
uint16_t ReadConfig16(uint8_t bus,
                      uint8_t device,
                      uint8_t function,
                      uint8_t offset) {
  uint32_t full = ReadConfig(bus, device, function, offset);
  if (offset & 2) {
    return (uint16_t)(full & 0xFFFF);
  } else {
    return (uint16_t)(full >> 16);
  }
}
uint8_t ReadConfig8(uint8_t bus,
                    uint8_t device,
                    uint8_t function,
                    uint8_t offset) {
  uint16_t full = ReadConfig16(bus, device, function, offset);
  if (offset & 1) {
    return (uint8_t)(full & 0xFF);
  } else {
    return (uint8_t)(full >> 8);
  }
}

void WriteConfig(uint8_t bus,
                 uint8_t device,
                 uint8_t function,
                 uint8_t offset,
                 uint32_t new_value) {
  uint32_t address = 0;
  ConfigCommand* cmd = (ConfigCommand*)&address;
  cmd->enable = 1;
  cmd->bus_number = bus;
  cmd->device_number = device;
  cmd->function_number = function;
  cmd->register_number = offset & 0xFC;

  outl(0xCF8, address);
  outl(0xCFC, new_value);
}

void WriteConfig16(uint8_t bus,
                   uint8_t device,
                   uint8_t function,
                   uint8_t offset,
                   uint16_t new_value) {
  uint32_t original = ReadConfig(bus, device, function, offset);
  uint32_t new_32 = 0;
  if (offset & 2) {
    // take left of original, right of new
    new_32 = (original & 0xFFFF0000) | (0x0000FFFF & (uint32_t)new_value);
  } else {
    new_32 =
        (original & 0x0000FFFF) | (0xFFFF0000 & (((uint32_t)new_value) << 16));
  }
  WriteConfig(bus, device, function, offset, new_32);
}

void WriteConfig8(uint8_t bus,
                  uint8_t device,
                  uint8_t function,
                  uint8_t offset,
                  uint8_t new_value) {
  uint16_t original = ReadConfig16(bus, device, function, offset);
  uint16_t new_16 = 0;
  if (offset & 1) {
    new_16 = (original & 0xFF00) | (0x00FF & (uint16_t)new_value);
  } else {
    new_16 = (original & 0x00FF) | (0xFF00 & (((uint16_t)new_value) << 8));
  }
  WriteConfig16(bus, device, function, offset, new_16);
}

stdj::Array<DeviceInfo> GetDevices() {
  stdj::Array<DeviceInfo> devices;
  DeviceInfo device;
  for (device.bus = 0; device.bus < 256; device.bus++) {
    for (device.device = 0; device.device < 32; device.device++) {
      device.vendor_id =
          ReadConfig16((uint8_t)device.bus, device.device, 0, OFFSET_VENDOR_ID);
      if (device.vendor_id != 0xFFFF) {
        device.device_id = ReadConfig16((uint8_t)device.bus, device.device, 0,
                                        OFFSET_DEVICE_ID);
        devices.Add(device);
      }
    }
  }
  return devices;
}

void PrintDeviceInfo(DeviceInfo device) {
  printk(
      "  status: 0x%04X, command: 0x%04X, bist: 0x%02X, headertype: 0x%02X\n",
      ReadConfig16(device.bus, device.device, 0, OFFSET_STATUS),
      ReadConfig16(device.bus, device.device, 0, OFFSET_COMMAND),
      ReadConfig8(device.bus, device.device, 0, OFFSET_BIST),
      ReadConfig8(device.bus, device.device, 0, OFFSET_HEADER_TYPE));
  printk("  classcode: 0x%02X, subclass: 0x%02X, progIF: 0x%02X, rev: 0x%02X\n",
         ReadConfig8(device.bus, device.device, 0, OFFSET_CLASS_CODE),
         ReadConfig8(device.bus, device.device, 0, OFFSET_SUBCLASS),
         ReadConfig8(device.bus, device.device, 0, OFFSET_PROG_IF),
         ReadConfig8(device.bus, device.device, 0, OFFSET_REVISION_ID));
  printk("  BAR0: 0x%08X, BAR1: 0x%08X, BAR2: 0x%08X\n",
         ReadConfig(device.bus, device.device, 0, OFFSET_BAR0),
         ReadConfig(device.bus, device.device, 0, OFFSET_BAR1),
         ReadConfig(device.bus, device.device, 0, OFFSET_BAR2));
  printk("  interrupt pin: 0x%02X, interrupt line: 0x%02X\n",
         ReadConfig8(device.bus, device.device, 0, OFFSET_INTERRUPT_PIN),
         ReadConfig8(device.bus, device.device, 0, OFFSET_INTERRUPT_LINE));
}

}  // namespace pci

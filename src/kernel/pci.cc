#include "pci.h"

#include "asm.h"
#include "printk.h"
#include "jarray.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define VENDOR_INTEL 0x8086
#define DEVICE_E1000 0x100E

// TODO why do i have to reverse the order of the fields?
struct ConfigCommand {
  uint32_t register_number : 8;  // bottom two bits must be zero
  uint32_t function_number : 3;
  uint32_t device_number : 5;
  uint32_t bus_number : 8;
  uint32_t reserved : 7;  // used in pci-express?
  uint32_t enable : 1;    // determines when accesses to CONFIG_DATA should be
                          // translated to configuration cycles
} __attribute__((packed));
static_assert(sizeof(ConfigCommand) == sizeof(uint32_t),
              "bad ConfigCommand size");

struct DeviceInfo {
  uint16_t bus;  // uint16_t instead of uint8_t for 256 loop hack
  uint8_t device;
  uint16_t vendor_id;
  uint16_t device_id;
};

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
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  return (uint16_t)(full >> ((offset & 2) * 8)) & 0xFFFF;
}

stdj::Array<DeviceInfo> GetDeviceInfo() {
  stdj::Array<DeviceInfo> devices;
  DeviceInfo device;
  for (device.bus = 0; device.bus < 256; device.bus++) {
    for (device.device = 0; device.device < 32; device.device++) {
      device.vendor_id = ReadConfig16((uint8_t)device.bus, device.device, 0, 0);
      if (device.vendor_id != 0xFFFF) {
        device.device_id =
            ReadConfig16((uint8_t)device.bus, device.device, 0, 2);
        devices.Add(device);
      }
    }
  }
  return devices;
}

void InitPci() {
  printk("InitPci()\n");
  stdj::Array<DeviceInfo> devices = GetDeviceInfo();
  for (int i = 0; i < devices.Size(); i++) {
    DeviceInfo device = devices.Get(i);
    printk("bus: 0x%X, device: 0x%X, vendor_id: 0x%X, device_id: 0x%X\n",
           device.bus, device.device, device.vendor_id, device.device_id);
  }
  //printk("bus 0 device 0 32: %p\n", ReadConfig(0, 0, 0, 0));
}

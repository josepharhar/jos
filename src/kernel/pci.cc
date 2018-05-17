#include "pci.h"

#include "asm.h"
#include "printk.h"
#include "jarray.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define VENDOR_INTEL 0x8086
#define DEVICE_E1000 0x100E

// offsets into the 256 byte configuration space, in bytes
#define OFFSET_DEVICE_ID 0x0        // 2 bytes
#define OFFSET_VENDOR_ID 0x2        // 2 bytes
#define OFFSET_STATUS 0x4           // 2 bytes
#define OFFSET_COMMAND 0x6          // 2 bytes
#define OFFSET_CLASS_CODE 0x8       // 1 byte
#define OFFSET_SUBCLASS 0x9         // 1 byte
#define OFFSET_PROG_IF 0xA          // 1 byte
#define OFFSET_REVISION_ID 0xB      // 1 byte
#define OFFSET_BIST 0xC             // 1 byte
#define OFFSET_HEADER_TYPE 0xD      // 1 byte
#define OFFSET_LATENCY_TIMER 0xE    // 1 byte
#define OFFSET_CACHE_LINE_SIZE 0xF  // 1 byte
#define OFFSET_BAR0 0x10            // 4 bytes
#define OFFSET_BAR1 0x14            // 4 bytes
#define OFFSET_BAR2 0x18            // 4 bytes
#define OFFSET_BAR3 0x1C            // 4 bytes
#define OFFSET_BAR4 0x20            // 4 bytes
#define OFFSET_BAR5 0x24            // 4 bytes
#define OFFSET_INTERRUPT_PIN 0x3E   // 1 byte
#define OFFSET_INTERRUPT_LINE 0x3F  // 1 byte

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

stdj::Array<DeviceInfo> GetDeviceInfo() {
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

void InitPci() {
  printk("InitPci()\n");
  stdj::Array<DeviceInfo> devices = GetDeviceInfo();
  for (int i = 0; i < devices.Size(); i++) {
    DeviceInfo device = devices.Get(i);
    /*printk("bus: 0x%X, device: 0x%X, vendor_id: 0x%04X, device_id: 0x%04X\n",
           device.bus, device.device, device.vendor_id, device.device_id);*/
    if (device.vendor_id == VENDOR_INTEL && device.device_id == DEVICE_E1000) {
      printk("found e1000 in bus %d, device %d\n", device.bus, device.device);
      printk(
          "  status: 0x%04X, command: 0x%04X, bist: 0x%02X, headertype: "
          "0x%02X\n",
          ReadConfig16(device.bus, device.device, 0, OFFSET_STATUS),
          ReadConfig16(device.bus, device.device, 0, OFFSET_COMMAND),
          ReadConfig8(device.bus, device.device, 0, OFFSET_BIST),
          ReadConfig8(device.bus, device.device, 0, OFFSET_HEADER_TYPE));
      printk(
          "  classcode: 0x%02X, subclass: 0x%02X, progIF: 0x%02X, rev: "
          "0x%02X\n",
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
      for (int j = 0; j < 0x18; j += 0x04) {
        if (j == 0x38 || j == 0x34 || j == 0x30 || j == 0x28) {
          break;
        }
        printk("[0x%02X] 0x%08X\n", j,
               ReadConfig(device.bus, device.device, 0, j));
      }
    }
  }
  // printk("bus 0 device 0 32: %p\n", ReadConfig(0, 0, 0, 0));
}

#ifndef KERNEL_PCI_H_
#define KERNEL_PCI_H_

#include "jarray.h"

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

// bits for command register
#define COMMAND_PIO 1
#define COMMAND_MMIO (1 << 1)
#define COMMAND_BUS_MASTER (1 << 2)
#define COMMAND_SPECIAL_CYCLES (1 << 3)
#define COMMAND_WRITE_AND_INVALIDATE (1 << 4)
#define COMMAND_VGA_SNOOP (1 << 5)
#define COMMAND_PARITY_ERROR (1 << 6)
#define COMMAND_RESERVED (1 << 7)
#define COMMAND_SERR (1 << 8)
#define COMMAND_FAST_BACK (1 << 9)
#define COMMAND_INTERRUPT_DISABLE (1 << 10)

namespace pci {

struct DeviceInfo {
  uint16_t bus;  // uint16_t instead of uint8_t for 256 loop hack
  uint8_t device;
  uint16_t vendor_id;
  uint16_t device_id;
};

uint32_t ReadConfig(uint8_t bus,
                    uint8_t device,
                    uint8_t function,
                    uint8_t offset);
uint16_t ReadConfig16(uint8_t bus,
                      uint8_t device,
                      uint8_t function,
                      uint8_t offset);
uint8_t ReadConfig8(uint8_t bus,
                    uint8_t device,
                    uint8_t function,
                    uint8_t offset);
void WriteConfig(uint8_t bus,
                 uint8_t device,
                 uint8_t function,
                 uint8_t offset,
                 uint32_t new_value);
void WriteConfig16(uint8_t bus,
                   uint8_t device,
                   uint8_t function,
                   uint8_t offset,
                   uint16_t new_value);
void WriteConfig8(uint8_t bus,
                  uint8_t device,
                  uint8_t function,
                  uint8_t offset,
                  uint8_t new_value);

stdj::Array<DeviceInfo> GetDevices();

void PrintDeviceInfo(DeviceInfo device);

}  // namespace pci

#endif  // KERNEL_PCI_H_

#ifndef KERNEL_E1000_H_
#define KERNEL_E1000_H_

#include "stdint.h"
#include "net.h"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

namespace net {

class E1000 {
 public:
  E1000(uint64_t interrupt_number, uint32_t bar, PacketReceivedHandler handler);

  bool start();
  void HandleInterrupt();
  uint8_t* getMacAddress();
  int sendPacket(const void* p_data, uint16_t p_len);  // Send a packet

 private:
  uint8_t bar_type;     // Type of BOR0
  uint16_t io_base;     // IO Base Address
  uint64_t mem_base;    // MMIO Base Address
  bool eerprom_exists;  // A flag indicating if eeprom exists
  uint8_t mac[6];       // A buffer for storing the mack address
  struct e1000_rx_desc*
      rx_descs[E1000_NUM_RX_DESC];  // Receive Descriptor Buffers
  struct e1000_tx_desc*
      tx_descs[E1000_NUM_TX_DESC];  // Transmit Descriptor Buffers
  void* tx_frames[E1000_NUM_TX_DESC];
  uint16_t rx_cur;  // Current Receive Descriptor Buffer
  uint16_t tx_cur;  // Current Transmit Descriptor Buffer
  PacketReceivedHandler handler_;

  uint64_t interrupt_number_;

  // Send Commands and read results From NICs either using MMIO or IO Ports
  void writeCommand(uint16_t p_address, uint32_t p_value);
  uint32_t readCommand(uint16_t p_address);

  bool detectEEProm();  // Return true if EEProm exist, else it returns false
                        // and set the eerprom_existsdata member
  uint32_t eepromRead(
      uint8_t addr);      // Read 4 bytes from a specific EEProm Address
  bool readMACAddress();  // Read MAC Address
  // void startLink();        // Start up the network
  void rxinit();           // Initialize receive descriptors an buffers
  void txinit();           // Initialize transmit descriptors an buffers
  void enableInterrupt();  // Enable Interrupts
  void handleReceive();    // Handle a packet reception.
};

}  // namespace net

#endif  // KERNEL_E1000_H_

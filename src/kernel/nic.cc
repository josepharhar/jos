#include "nic.h"

#include "frame.h"
#include "printk.h"
#include "irq.h"

#define INTEL_VEND 0x8086  // Vendor ID for Intel
#define E1000_DEV \
  0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217 0x153A     // Device ID for Intel I217
#define E1000_82577LM 0x10EA  // Device ID for Intel 82577LM

// I have gathered those from different Hobby online operating systems instead
// of getting them one by one from the manual

#define REG_CTRL 0x0000
#define REG_STATUS 0x0008
#define REG_EEPROM 0x0014
#define REG_CTRL_EXT 0x0018
#define REG_IMASK 0x00D0
#define REG_RCTRL 0x0100
#define REG_RXDESCLO 0x2800
#define REG_RXDESCHI 0x2804
#define REG_RXDESCLEN 0x2808
#define REG_RXDESCHEAD 0x2810
#define REG_RXDESCTAIL 0x2818

#define REG_TCTRL 0x0400
#define REG_TXDESCLO 0x3800
#define REG_TXDESCHI 0x3804
#define REG_TXDESCLEN 0x3808
#define REG_TXDESCHEAD 0x3810
#define REG_TXDESCTAIL 0x3818

#define REG_RDTR 0x2820    // RX Delay Timer Register
#define REG_RXDCTL 0x3828  // RX Descriptor Control
#define REG_RADV 0x282C    // RX Int. Absolute Delay Timer
#define REG_RSRPD 0x2C00   // RX Small Packet Detect Interrupt

#define REG_TIPG 0x0410  // Transmit Inter Packet Gap
#define ECTRL_SLU 0x40   // set link up

#define RCTL_EN (1 << 1)             // Receiver Enable
#define RCTL_SBP (1 << 2)            // Store Bad Packets
#define RCTL_UPE (1 << 3)            // Unicast Promiscuous Enabled
#define RCTL_MPE (1 << 4)            // Multicast Promiscuous Enabled
#define RCTL_LPE (1 << 5)            // Long Packet Reception Enable
#define RCTL_LBM_NONE (0 << 6)       // No Loopback
#define RCTL_LBM_PHY (3 << 6)        // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF (0 << 8)     // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER (1 << 8)  // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH (2 << 8)   // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36 (0 << 12)         // Multicast Offset - bits 47:36
#define RCTL_MO_35 (1 << 12)         // Multicast Offset - bits 46:35
#define RCTL_MO_34 (2 << 12)         // Multicast Offset - bits 45:34
#define RCTL_MO_32 (3 << 12)         // Multicast Offset - bits 43:32
#define RCTL_BAM (1 << 15)           // Broadcast Accept Mode
#define RCTL_VFE (1 << 18)           // VLAN Filter Enable
#define RCTL_CFIEN (1 << 19)         // Canonical Form Indicator Enable
#define RCTL_CFI (1 << 20)           // Canonical Form Indicator Bit Value
#define RCTL_DPF (1 << 22)           // Discard Pause Frames
#define RCTL_PMCF (1 << 23)          // Pass MAC Control Frames
#define RCTL_SECRC (1 << 26)         // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256 (3 << 16)
#define RCTL_BSIZE_512 (2 << 16)
#define RCTL_BSIZE_1024 (1 << 16)
#define RCTL_BSIZE_2048 (0 << 16)
#define RCTL_BSIZE_4096 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384 ((1 << 16) | (1 << 25))

// Transmit Command

#define CMD_EOP (1 << 0)   // End of Packet
#define CMD_IFCS (1 << 1)  // Insert FCS
#define CMD_IC (1 << 2)    // Insert Checksum
#define CMD_RS (1 << 3)    // Report Status
#define CMD_RPS (1 << 4)   // Report Packet Sent
#define CMD_VLE (1 << 6)   // VLAN Packet Enable
#define CMD_IDE (1 << 7)   // Interrupt Delay Enable

// TCTL Register

#define TCTL_EN (1 << 1)       // Transmit Enable
#define TCTL_PSP (1 << 3)      // Pad Short Packets
#define TCTL_CT_SHIFT 4        // Collision Threshold
#define TCTL_COLD_SHIFT 12     // Collision Distance
#define TCTL_SWXOFF (1 << 22)  // Software XOFF Transmission
#define TCTL_RTLC (1 << 24)    // Re-transmit on Late Collision

#define TSTA_DD (1 << 0)  // Descriptor Done
#define TSTA_EC (1 << 1)  // Excess Collisions
#define TSTA_LC (1 << 2)  // Late Collision
#define LSTA_TU (1 << 3)  // Transmit Underrun

struct e1000_rx_desc {
  volatile uint64_t addr;
  volatile uint16_t length;
  volatile uint16_t checksum;
  volatile uint8_t status;
  volatile uint8_t errors;
  volatile uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
  volatile uint64_t addr;
  volatile uint16_t length;
  volatile uint8_t cso;
  volatile uint8_t cmd;
  volatile uint8_t status;
  volatile uint8_t css;
  volatile uint16_t special;
} __attribute__((packed));

class MMIOUtils {
 public:
  static uint8_t read8(uint64_t p_address);
  static uint16_t read16(uint64_t p_address);
  static uint32_t read32(uint64_t p_address);
  static uint64_t read64(uint64_t p_address);
  static void write8(uint64_t p_address, uint8_t p_value);
  static void write16(uint64_t p_address, uint16_t p_value);
  static void write32(uint64_t p_address, uint32_t p_value);
  static void write64(uint64_t p_address, uint64_t p_value);
};

uint8_t MMIOUtils::read8(uint64_t p_address) {
  return *((volatile uint8_t*)(p_address));
}
uint16_t MMIOUtils::read16(uint64_t p_address) {
  return *((volatile uint16_t*)(p_address));
}
uint32_t MMIOUtils::read32(uint64_t p_address) {
  return *((volatile uint32_t*)(p_address));
}
uint64_t MMIOUtils::read64(uint64_t p_address) {
  return *((volatile uint64_t*)(p_address));
}
void MMIOUtils::write8(uint64_t p_address, uint8_t p_value) {
  (*((volatile uint8_t*)(p_address))) = (p_value);
}
void MMIOUtils::write16(uint64_t p_address, uint16_t p_value) {
  (*((volatile uint16_t*)(p_address))) = (p_value);
}
void MMIOUtils::write32(uint64_t p_address, uint32_t p_value) {
  (*((volatile uint32_t*)(p_address))) = (p_value);
}
void MMIOUtils::write64(uint64_t p_address, uint64_t p_value) {
  (*((volatile uint64_t*)(p_address))) = (p_value);
}

class Ports {
 public:
  static void outportb(uint16_t p_port, uint8_t data);
  static void outportw(uint16_t p_port, uint16_t data);
  static void outportl(uint16_t p_port, uint32_t data);
  static uint8_t inportb(uint16_t p_port);
  static uint16_t inportw(uint16_t p_port);
  static uint32_t inportl(uint16_t p_port);
};

/* void Ports::outportb (uint16_t p_port,uint8_t p_data)
 *
 * This method outputs a byte to a hardware port.
 * It uses an inline asm with the volatile keyword
 * to disable compiler optimization.
 *
 *  p_port: the port number to output the byte p_data to.
 *  p_data: the byte to to output to the port p_port.
 *
 * Notice the input constraint
 *      "dN" (port) : indicates using the DX register to store the
 *                  value of port in it
 *      "a"  (data) : store the value of data into
 *
 * The above constraint will instruct the compiler to generate assembly
 * code that looks like that
 *      mov    %edi,%edx
 *      mov    %esi,%eax
 *      out    %eax,(%dx)
 *
 * According the ABI, the edi will have the value of p_port and esi will have
 * the value of the p_data
 *
 */
void Ports::outportb(uint16_t p_port, uint8_t p_data) {
  asm volatile("outb %1, %0" : : "dN"(p_port), "a"(p_data));
}

/* void Ports::outportw (uint16_t p_port,uint16_t p_data)
 *
 * This method outputs a word to a hardware port.
 *
 *  p_port: the port number to output the byte p_data to.
 *  p_data: the byte to to output to the port p_port.
 *
 */
void Ports::outportw(uint16_t p_port, uint16_t p_data) {
  asm volatile("outw %1, %0" : : "dN"(p_port), "a"(p_data));
}

/* void Ports::outportl (uint16_t p_port,uint32_t p_data)
 *
 * This method outputs a double word to a hardware port.
 *
 *  p_port: the port number to output the byte p_data to.
 *  p_data: the byte to to output to the port p_port.
 *
 */
void Ports::outportl(uint16_t p_port, uint32_t p_data) {
  asm volatile("outl %1, %0" : : "dN"(p_port), "a"(p_data));
}

/* uint8_t Ports::inportb( uint16_t p_port)
 *
 * This method reads a byte from a hardware port.
 *
 *  p_port: the port number to read the byte from.
 *  return value : a byte read from the port p_port.
 *
 * Notice the output constraint "=a", this tells the compiler
 * to expect the save the value of register AX into the variable l_ret
 * The register AX should contain the result of the inb instruction.
 *
 *
 */
uint8_t Ports::inportb(uint16_t p_port) {
  uint8_t l_ret;
  asm volatile("inb %1, %0" : "=a"(l_ret) : "dN"(p_port));
  return l_ret;
}

/* uint16_t Ports::inportw( uint16_t p_port)
 *
 * This method reads a word from a hardware port.
 *
 *  p_port: the port number to read the word from.
 *  return value : a word read from the port p_port.
 *
 */
uint16_t Ports::inportw(uint16_t p_port) {
  uint16_t l_ret;
  asm volatile("inw %1, %0" : "=a"(l_ret) : "dN"(p_port));
  return l_ret;
}

/* uint16_t Ports::inportl( uint16_t p_port)
 *
 * This method reads a double word from a hardware port.
 *
 *  p_port: the port number to read the double word from.
 *  return value : a double word read from the port p_port.
 *
 */
uint32_t Ports::inportl(uint16_t p_port) {
  uint32_t l_ret;
  asm volatile("inl %1, %0" : "=a"(l_ret) : "dN"(p_port));
  return l_ret;
}

void E1000::writeCommand(uint16_t p_address, uint32_t p_value) {
  if (bar_type == 0) {
    MMIOUtils::write32(mem_base + p_address, p_value);
  } else {
    Ports::outportl(io_base, p_address);
    Ports::outportl(io_base + 4, p_value);
  }
}

uint32_t E1000::readCommand(uint16_t p_address) {
  if (bar_type == 0) {
    return MMIOUtils::read32(mem_base + p_address);
  } else {
    Ports::outportl(io_base, p_address);
    return Ports::inportl(io_base + 4);
  }
}

bool E1000::detectEEProm() {
  uint32_t val = 0;
  writeCommand(REG_EEPROM, 0x1);

  for (int i = 0; i < 1000 && !eerprom_exists; i++) {
    val = readCommand(REG_EEPROM);
    if (val & 0x10)
      eerprom_exists = true;
    else
      eerprom_exists = false;
  }
  return eerprom_exists;
}

uint32_t E1000::eepromRead(uint8_t addr) {
  uint16_t data = 0;
  uint32_t tmp = 0;
  if (eerprom_exists) {
    writeCommand(REG_EEPROM, (1) | ((uint32_t)(addr) << 8));
    while (!((tmp = readCommand(REG_EEPROM)) & (1 << 4)))
      ;
  } else {
    writeCommand(REG_EEPROM, (1) | ((uint32_t)(addr) << 2));
    while (!((tmp = readCommand(REG_EEPROM)) & (1 << 1)))
      ;
  }
  data = (uint16_t)((tmp >> 16) & 0xFFFF);
  return data;
}

bool E1000::readMACAddress() {
  if (eerprom_exists) {
    uint32_t temp;
    temp = eepromRead(0);
    mac[0] = temp & 0xff;
    mac[1] = temp >> 8;
    temp = eepromRead(1);
    mac[2] = temp & 0xff;
    mac[3] = temp >> 8;
    temp = eepromRead(2);
    mac[4] = temp & 0xff;
    mac[5] = temp >> 8;
  } else {
    uint8_t* mem_base_mac_8 = (uint8_t*)(mem_base + 0x5400);
    uint32_t* mem_base_mac_32 = (uint32_t*)(mem_base + 0x5400);
    if (mem_base_mac_32[0] != 0) {
      for (int i = 0; i < 6; i++) {
        mac[i] = mem_base_mac_8[i];
      }
    } else {
      return false;
    }
  }
  return true;
}

void E1000::rxinit() {
  uint8_t* ptr;
  struct e1000_rx_desc* descs;

  // Allocate buffer for receive descriptors. For simplicity, in my case
  // khmalloc returns a virtual address that is identical to it physical mapped
  // address.
  // In your case you should handle virtual and physical addresses as the
  // addresses passed to the NIC should be physical ones

  /*ptr = (uint8_t*)(kmalloc_ptr->khmalloc(
      sizeof(struct e1000_rx_desc) * E1000_NUM_RX_DESC + 16));*/
  // 512 + 16 bytes, less than one frame (4096)!
  int num_bytes = sizeof(e1000_rx_desc) * E1000_NUM_RX_DESC + 16;
  ptr = (uint8_t*)FrameAllocate();

  // hope to god that first FrameAllocate() calls in kernel initialization are
  // contiguous?
  uint64_t frame_one = (uint64_t)FrameAllocate();
  uint64_t frame_two = (uint64_t)FrameAllocate();
  uint64_t frame_three = (uint64_t)FrameAllocate();
  if (!(frame_two == frame_one + (4096 / sizeof(uint64_t)) &&
        frame_three == frame_two + (4096 / sizeof(uint64_t)))) {
    printk("E1000::rxinit FRAMES ARE NOT CONTIGUOUS!!!\n");
    printk("  frame_one: %p\n", frame_one);
    printk("  frame_two: %p\n", frame_two);
    printk("  frame_three: %p\n", frame_three);
  }

  descs = (struct e1000_rx_desc*)ptr;
  for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
    rx_descs[i] = (struct e1000_rx_desc*)((uint8_t*)descs + i * 16);
    // rx_descs[i]->addr = (uint64_t)(uint8_t*)(kmalloc_ptr->khmalloc(8192 +
    // 16));
    rx_descs[i]->addr = frame_one;
    rx_descs[i]->status = 0;
  }

  writeCommand(REG_TXDESCLO, (uint32_t)((uint64_t)ptr >> 32));
  writeCommand(REG_TXDESCHI, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

  writeCommand(REG_RXDESCLO, (uint64_t)ptr);
  writeCommand(REG_RXDESCHI, 0);

  writeCommand(REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

  writeCommand(REG_RXDESCHEAD, 0);
  writeCommand(REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);
  rx_cur = 0;
  writeCommand(REG_RCTRL, RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE |
                              RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM |
                              RCTL_SECRC | RCTL_BSIZE_2048);
}

void E1000::txinit() {
  uint8_t* ptr;
  struct e1000_tx_desc* descs;
  // Allocate buffer for receive descriptors. For simplicity, in my case
  // khmalloc returns a virtual address that is identical to it physical mapped
  // address.
  // In your case you should handle virtual and physical addresses as the
  // addresses passed to the NIC should be physical ones
  /*ptr = (uint8_t*)(kmalloc_ptr->khmalloc(
      sizeof(struct e1000_tx_desc) * E1000_NUM_TX_DESC + 16));*/
  ptr = (uint8_t*)FrameAllocate();

  descs = (struct e1000_tx_desc*)ptr;
  for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
    tx_descs[i] = (struct e1000_tx_desc*)((uint8_t*)descs + i * 16);
    tx_descs[i]->addr = 0;
    tx_descs[i]->cmd = 0;
    tx_descs[i]->status = TSTA_DD;
  }

  writeCommand(REG_TXDESCHI, (uint32_t)((uint64_t)ptr >> 32));
  writeCommand(REG_TXDESCLO, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

  // now setup total length of descriptors
  writeCommand(REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

  // setup numbers
  writeCommand(REG_TXDESCHEAD, 0);
  writeCommand(REG_TXDESCTAIL, 0);
  tx_cur = 0;
  writeCommand(REG_TCTRL, TCTL_EN | TCTL_PSP | (15 << TCTL_CT_SHIFT) |
                              (64 << TCTL_COLD_SHIFT) | TCTL_RTLC);

  // This line of code overrides the one before it but I left both to highlight
  // that the previous one works with e1000 cards, but for the e1000e cards
  // you should set the TCTRL register as follows. For detailed description of
  // each bit, please refer to the Intel Manual.
  // In the case of I217 and 82577LM packets will not be sent if the TCTRL is
  // not configured using the following bits.
  writeCommand(REG_TCTRL, 0b0110000000000111111000011111010);
  writeCommand(REG_TIPG, 0x0060200A);
}

void E1000::enableInterrupt() {
  writeCommand(REG_IMASK, 0x1F6DC);
  writeCommand(REG_IMASK, 0xff & ~4);
  readCommand(0xc0);
}

/*E1000::E1000(PCIConfigHeader* p_pciConfigHeader)
    : NetworkDriver(p_pciConfigHeader) {*/
E1000::E1000(uint64_t interrupt_number, uint32_t bar)
    : interrupt_number_(interrupt_number) {
  io_base = 0;
  mem_base = 0;
  if (bar & 1) {
    // port based io
    bar_type = 1;
    io_base = bar & 0xFFFFFFFC;
  } else {
    // memory mapped io
    bar_type = 0;
    mem_base = bar & 0xFFFFFFF0;
  }
  // Get BAR0 type, io_base address and MMIO base address
  /*bar_type = pciConfigHeader->getPCIBarType(0);
  io_base = pciConfigHeader->getPCIBar(PCI_BAR_IO) & ~1;
  mem_base = pciConfigHeader->getPCIBar(PCI_BAR_MEM) & ~3;*/

  // Enable bus mastering
  //pciConfigHeader->enablePCIBusMastering();
  eerprom_exists = false;
}

static void GlobalInterruptHandler(uint64_t interrupt_number, void* arg) {
  printk("network GlobalInterruptHandler, interrupt_number: %p\n");
  E1000* instance = (E1000*)arg;
  instance->HandleInterrupt();
}

bool E1000::start() {
  detectEEProm();
  if (!readMACAddress())
    return false;
  printk("E1000::start() got mac: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],
         mac[1], mac[2], mac[3], mac[4], mac[5]);
  // printMac();
  // startLink();

  for (int i = 0; i < 0x80; i++)
    writeCommand(0x5200 + i * 4, 0);
  /*if (interruptManager->registerInterrupt(IRQ0 +
     pciConfigHeader->getIntLine(),
                                          this)) {*/
  IRQSetHandler(GlobalInterruptHandler, interrupt_number_, this);
  enableInterrupt();
  rxinit();
  txinit();
  printk("E1000 driver started\n");
  return true;
  /*} else
    return false;*/
}

// void E1000::fire(InterruptContext* p_interruptContext) {
void E1000::HandleInterrupt() {
  /*if (p_interruptContext->getInteruptNumber() ==
      pciConfigHeader->getIntLine() + IRQ0) {*/
  /* This might be needed here if your handler doesn't clear interrupts from
     each device and must be done before EOI if using the PIC.
     Without this, the card will spam interrupts as the int-line will stay
     high. */
  writeCommand(REG_IMASK, 0x1);

  uint32_t status = readCommand(0xc0);
  if (status & 0x04) {
    // TODO reset network stack?
    printk("TODO reset network stack?\n");
    // startLink();
  } else if (status & 0x10) {
    // good threshold
  } else if (status & 0x80) {
    handleReceive();
  }
  //}
}

void E1000::handleReceive() {
  uint16_t old_cur;
  bool got_packet = false;

  while ((rx_descs[rx_cur]->status & 0x1)) {
    got_packet = true;
    uint8_t* buf = (uint8_t*)rx_descs[rx_cur]->addr;
    uint16_t len = rx_descs[rx_cur]->length;

    // Here you should inject the received packet into your network stack

    rx_descs[rx_cur]->status = 0;
    old_cur = rx_cur;
    rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
    writeCommand(REG_RXDESCTAIL, old_cur);
  }
}

int E1000::sendPacket(const void* p_data, uint16_t p_len) {
  tx_descs[tx_cur]->addr = (uint64_t)p_data;
  tx_descs[tx_cur]->length = p_len;
  tx_descs[tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS;
  tx_descs[tx_cur]->status = 0;
  uint8_t old_cur = tx_cur;
  tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
  writeCommand(REG_TXDESCTAIL, tx_cur);
  while (!(tx_descs[old_cur]->status & 0xff))
    ;
  return 0;
}

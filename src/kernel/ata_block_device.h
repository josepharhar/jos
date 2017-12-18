#ifndef ATA_BLOCK_DEVICE_H_
#define ATA_BLOCK_DEVICE_H_

// http://wiki.osdev.org/ATA_PIO_Mode

#include "block_device.h"
#include "queue.h"
#include "proc.h"

#define PRIMARY_ATA_PORT 0x1F0
#define PRIMARY_STATUS_PORT 0x3F6 // "alternate" status port
#define PRIMARY_DEVICE_CONTROL_PORT 0x3F6 // 0x3F6 doubles as status read and control write

// port offsets from PRIMARY_ATA_PORT
#define PORT_OFFSET_DATA 0
#define PORT_OFFSET_FEATURES 1
#define PORT_OFFSET_SECTOR_COUNT 2
#define PORT_OFFSET_LBA_LO 3
#define PORT_OFFSET_LBA_MID 4
#define PORT_OFFSET_LBA_HI 5
#define PORT_OFFSET_DRIVE_SELECT 6 // use to select a drive
#define PORT_OFFSET_STATUS 7 // "regular" status port, also command port
#define PORT_OFFSET_CMD 7 // register 7 doubles as status read and cmd write

// bit masks from PORT_OFFSET_STATUS/PRIMARY_STATUS_PORT
#define STATUS_ERR 1 // indicates an error occured
#define STATUS_DRQ (1 << 3) // set when drive has PIO data to give or receive
#define STATUS_SRV (1 << 4) // overlapped mode service request
#define STATUS_DF (1 << 5) // drive fault error (does not set ERR)
#define STATUS_RDY (1 << 6) // clear when drive is spun down or after an error
#define STATUS_BSY (1 << 7) // set when drive is preparing to send/receive data. overrides other bits

// bit masks for PRIMARY_DEVICE_CONTROL_PORT
#define CONTROL_NIEN (1 << 1) // set to stop current device from sending interrupts
#define CONTROL_SRST (1 << 2) // set to do a software reset on all drives on the bus
#define CONTROL_HOB (1 << 7) // set to read back high order byte of last lab48 value sent on io port

struct ATARequest {
  enum class RequestType { READ, WRITE };

  uint64_t block_num;
  void* buffer;
  RequestType request_type;
};

class ATABlockDevice : public BlockDevice {
 public:
  ~ATABlockDevice();

  // buffers must be in kernel memory so interrupt handler can use it
  // after a context swap out of this process
  int ReadBlock(uint64_t block_num, void* dest) override;
  int WriteBlock(uint64_t block_num, void* src) override;

  static ATABlockDevice* Probe(uint16_t bus_base_port,
                               uint16_t ata_master,
                               bool is_master,
                               const char* name,
                               uint8_t irq);
  static void Destroy(ATABlockDevice* ata_device);

  static void GlobalInterruptHandler(uint64_t interrupt_number, void* arg);

 private:
  ATABlockDevice();

  void InterruptHandler();
  void Consume(struct ATARequest* request);

  //uint16_t ata_base; // ata controller base address
  uint16_t bus_base_port; // ata controller base address? same as ata_base?
  uint16_t ata_master; // ata controller master address TODO what is this
  bool is_master; // flag, master = 1, slave = 0
  uint8_t irq; // irq # for this controller TODO how do i use this
  Proc::BlockedQueue* proc_queue;
  uint64_t num_sectors;
  Queue<ATARequest> request_queue;
};

#endif  // ATA_BLOCK_DEVICE_H_

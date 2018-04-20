#include "ata.h"

#include "shared/jqueue.h"

#include "kernel/kmalloc.h"
#include "kernel/printk.h"
#include "kernel/irq.h"

// http://wiki.osdev.org/ATA_PIO_Mode

#define PRIMARY_STATUS_PORT 0x3F6  // "alternate" status port
#define PRIMARY_DEVICE_CONTROL_PORT \
  0x3F6  // 0x3F6 doubles as status read and control write

// port offsets from PRIMARY_ATA_PORT
#define PORT_OFFSET_DATA 0
#define PORT_OFFSET_FEATURES 1
#define PORT_OFFSET_SECTOR_COUNT 2
#define PORT_OFFSET_LBA_LO 3
#define PORT_OFFSET_LBA_MID 4
#define PORT_OFFSET_LBA_HI 5
#define PORT_OFFSET_DRIVE_SELECT 6  // use to select a drive
#define PORT_OFFSET_STATUS 7        // "regular" status port, also command port
#define PORT_OFFSET_CMD 7  // register 7 doubles as status read and cmd write

// bit masks from PORT_OFFSET_STATUS/PRIMARY_STATUS_PORT
#define STATUS_ERR 1         // indicates an error occured
#define STATUS_DRQ (1 << 3)  // set when drive has PIO data to give or receive
#define STATUS_SRV (1 << 4)  // overlapped mode service request
#define STATUS_DF (1 << 5)   // drive fault error (does not set ERR)
#define STATUS_RDY (1 << 6)  // clear when drive is spun down or after an error
#define STATUS_BSY \
  (1 << 7)  // set when drive is preparing to send/receive data. overrides other
            // bits

// bit masks for PRIMARY_DEVICE_CONTROL_PORT
#define CONTROL_NIEN \
  (1 << 1)  // set to stop current device from sending interrupts
#define CONTROL_SRST \
  (1 << 2)  // set to do a software reset on all drives on the bus
#define CONTROL_HOB \
  (1 << 7)  // set to read back high order byte of last lab48 value sent on io
            // port

namespace vfs {

static stdj::Queue<ATARequest> request_queue;

ATADevice::ATADevice() {}
ATADevice::~ATADevice() {
  // TODO unregister interrupt handler?
}

// static
void ATADevice::GlobalInterruptHandler(uint64_t interrupt_number, void* arg) {
  ATADevice* ata_device = (ATADevice*)arg;
  ata_device->InterruptHandler();
}

void ATADevice::InterruptHandler() {
  uint8_t status = inb(bus_base_port + PORT_OFFSET_STATUS);
  if ((status & STATUS_BSY) || !(status & STATUS_DRQ)) {
    printk("ATADevice::InterruptHandler() invalid status: 0x%X\n", status);
    return;
  }

  if (request_queue.IsEmpty()) {
    printk(
        "ATABlockDevice::InterruptHandler() called with no requests! status: "
        "0x%X\n",
        status);
    printk("dumping some input: ");
    int i;
    for (i = 0; inb(PRIMARY_ATA_PORT + PORT_OFFSET_STATUS) & STATUS_DRQ; i++) {
      uint16_t input = inw(PRIMARY_ATA_PORT);
      if (i < 3) {
        printk("0x%X 0x%X ", input & 0xFF, (input >> 8) & 0xFF);
      }
    }
    printk("\n");
    printk("dumped %d bytes\n\n", i * 2);
    return;
  }

  // service request
  ATARequest request = request_queue.Remove();

  switch (request.type) {
    case ATARequest::READ: {
      // this reading could take a long time.
      //   would it be possible to defer it until after the interrupt handler?

      uint16_t* buffer = (uint16_t*)request.buffer;
      uint16_t request_port = bus_base_port + PORT_OFFSET_DATA;
      for (int i = 0; i < 256; i++) {
        // inw() must be used to read from HDD
        // it reads two bytes at a time in big endian order,
        // so we must use buffer as a byte/char buffer instead of a
        // short/uint16_t
        // buffer
        buffer[i] = inw(request_port);
      }
      break;
    }
    default:
      printk("ATADevice::InterruptHandler unknown request.type: %D\n",
             request.type);
      break;
  }

  // consume another request if there is one
  if (!request_queue.IsEmpty()) {
    Consume(request_queue.Peek());
  }

  // signal that the request has been completed
  if (request.callback) {
    request.callback(request.callback_arg);
  }
}

static uint8_t PollStatus(uint16_t status_port) {
  // osdev says to read five times first
  uint8_t status = 0;
  for (int i = 0; i < 5 || (status & STATUS_BSY); i++) {
    status = inb(status_port);
    if (i > 100) {
      printk(
          "polled status 100 times but still busy. port: 0x%X, status: 0x%X\n",
          status_port, status);
      return status;
    }
  }
  return status;
}

int ATADevice::ReadBlock(uint64_t block_num,
                         void* dest,
                         ATARequestCallback callback,
                         void* callback_arg) {
  // create a new request and add it to request queue
  ATARequest new_request;
  new_request.block_num = block_num;
  new_request.buffer = dest;
  new_request.type = ATARequest::READ;
  new_request.callback = callback;
  new_request.callback_arg = callback_arg;
  bool queue_was_empty = request_queue.IsEmpty();
  request_queue.Add(new_request);

  // send READ SECTORS EXT command? - no send on consume, may or may not be now
  // if the request queue was empty and this request is the only one in it,
  // then service it now. otherwise, wait
  if (queue_was_empty) {
    Consume(new_request);
  } else {
    printk("queue was not empty, waiting to consume request block_num: %lu\n",
           new_request.block_num);
  }

  // block this process until interrupt happens and buffer is read into
  // proc_queue->BlockCurrentProc();

  return 0;
}

int ATADevice::WriteBlock(uint64_t block_num,
                          void* src,
                          ATARequestCallback callback,
                          void* callback_arg) {
  // TODO implement writing
  return 1;
}

// static
ATADevice* ATADevice::Probe(uint16_t bus_base_port,
                            uint16_t ata_master,
                            bool is_master,
                            const char* name,
                            uint8_t irq) {
  request_queue = stdj::Queue<ATARequest>();

  irq = 46;  // TODO

  // block interrupts during initialization
  IRQSetHandler(&IRQHandlerEmpty, irq, 0);

  uint8_t status = 0;

  // IDENTIFY command
  // send 0xA0 for master, 0xB0 for slave
  outb(bus_base_port + PORT_OFFSET_DRIVE_SELECT, 0xA0);
  // set LBAlo, LBAmid, and LBAhi to 0
  outb(bus_base_port + PORT_OFFSET_LBA_LO, 0);
  outb(bus_base_port + PORT_OFFSET_LBA_MID, 0);
  outb(bus_base_port + PORT_OFFSET_LBA_HI, 0);
  // send IDENTIFY command 0xEC to command port
  outb(bus_base_port + PORT_OFFSET_CMD, 0xEC);
  // read status after sending identify command
  status = inb(bus_base_port + PORT_OFFSET_STATUS);
  // if status is zero, then drive does not exist. else, read until not busy
  do {
    status = inb(bus_base_port + PORT_OFFSET_STATUS);
  } while (status & STATUS_BSY);
  if (!status) {
    printk("!status, drive does not exist. initialization failed\n");
    return 0;
  }

  // make sure LBAmid and LBAhi are zero
  if (inb(bus_base_port + PORT_OFFSET_LBA_MID) ||
      inb(bus_base_port + PORT_OFFSET_LBA_HI)) {
    printk("LBAmid | LBAhi, drive is not ATA. initialization failed\n");
    return 0;
  }

  // continue polling status port until DRQ or ERR is set
  do {
    status = inb(bus_base_port + PORT_OFFSET_STATUS);
  } while (!(status & STATUS_ERR) && !(status & STATUS_DRQ));
  if (status & STATUS_ERR) {
    printk("ERR bit set. initialization failed\n");
    return 0;
  }

  // data is ready to read from the data port. read 256 16bit values and store
  // them
  uint16_t* identify_buffer = (uint16_t*)kmalloc(256 * sizeof(uint16_t));
  for (int i = 0; i < 256; i++) {
    identify_buffer[i] = inw(bus_base_port + PORT_OFFSET_DATA);
  }
  if (!(identify_buffer[83] & (1 << 10))) {
    printk("LBA48 mode not supported, initialization failed\n");
    kfree(identify_buffer);
    return 0;
  }
  uint64_t num_sectors = 0;
  num_sectors |= (((uint64_t)identify_buffer[100]) << 0);
  num_sectors |= (((uint64_t)identify_buffer[101]) << 16);
  num_sectors |= (((uint64_t)identify_buffer[102]) << 32);
  num_sectors |= (((uint64_t)identify_buffer[103]) << 48);
  // printk("num LBA48 addressable sectors: %p\n", num_sectors);
  kfree(identify_buffer);

  ATADevice* device = new ATADevice();
  device->bus_base_port = bus_base_port;
  device->is_master = is_master;
  device->ata_master = ata_master;
  device->irq = irq;
  device->num_sectors = num_sectors;

  IRQSetHandler(&GlobalInterruptHandler, device->irq, device);

  return device;
}

void ATADevice::Consume(ATARequest request) {
  uint64_t lba = request.block_num;
  uint8_t sectorcount_low_byte = num_sectors & 0xFF;
  uint8_t sectorcount_high_byte = (num_sectors >> 8) & 0xFF;

  // sectorcount is number of 512 byte blocks to read
  // we never want to read more than one block at a time
  sectorcount_low_byte = 1;
  sectorcount_high_byte = 0;

  /*printk("ATADevice::Consume block_num: %p, buffer: %p\n",
      request.block_num, request.buffer);*/
  PollStatus(bus_base_port + PORT_OFFSET_STATUS);
  outb(bus_base_port + PORT_OFFSET_DRIVE_SELECT, 0x40);  // 0x40: master, lba48
  PollStatus(bus_base_port + PORT_OFFSET_STATUS);
  outb(bus_base_port + PORT_OFFSET_SECTOR_COUNT,
       sectorcount_high_byte);  // sectorcount high byte
  outb(bus_base_port + PORT_OFFSET_LBA_LO, (lba >> 24) & 0xFF);   // lba4
  outb(bus_base_port + PORT_OFFSET_LBA_MID, (lba >> 32) & 0xFF);  // lba5
  outb(bus_base_port + PORT_OFFSET_LBA_HI, (lba >> 48) & 0xFF);   // lba6
  PollStatus(bus_base_port + PORT_OFFSET_STATUS);
  outb(bus_base_port + PORT_OFFSET_SECTOR_COUNT,
       sectorcount_low_byte);                            // sectorcount low byte
  outb(bus_base_port + PORT_OFFSET_LBA_LO, lba & 0xFF);  // lba1
  outb(bus_base_port + PORT_OFFSET_LBA_MID, (lba >> 8) & 0xFF);  // lba2
  outb(bus_base_port + PORT_OFFSET_LBA_HI, (lba >> 16) & 0xFF);  // lba3
  outb(bus_base_port + PORT_OFFSET_CMD,
       0x24);  // send "READ SECTORS EXT" command 0x24
}

void ATADevice::PrintQueue() {
  printk("ATADevice::PrintQueue() request_queue.Size(): %d\n",
         request_queue.Size());
  for (int i = 0; i < request_queue.Size(); i++) {
    ATARequest request = request_queue.GetAtIndex(i);
    printk("  [%d]: block_num: %ld buffer: %p, callback: %p\n", i,
           request.block_num, request.buffer, request.callback);
  }
}

}  // namespace vfs

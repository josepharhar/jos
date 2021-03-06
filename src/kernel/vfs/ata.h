#ifndef VFS_ATA_H_
#define VFS_ATA_H_

#include "stdint.h"

#define PRIMARY_ATA_PORT 0x1F0

namespace vfs {

typedef void (*ATARequestCallback)(void*);

struct ATARequest {
  enum RequestType {
    READ,
  } type;
  uint64_t block_num;
  void* buffer;
  ATARequestCallback callback;
  void* callback_arg;
};

class ATADevice {
 public:
  ~ATADevice();

  // returns 0 on success, calls callback when request is done
  int ReadBlock(uint64_t block_num,
                void* dest,
                ATARequestCallback callback,
                void* callback_arg = 0);
  int WriteBlock(uint64_t block_num,
                 void* src,
                 ATARequestCallback callback,
                 void* callback_arg = 0);

  static ATADevice* Probe(uint16_t bus_base_port,
                          uint16_t ata_master,
                          bool is_master,
                          const char* name,
                          uint8_t irq);
  static void GlobalInterruptHandler(uint64_t interrupt_number, void* arg);

  void PrintQueue();

 private:
  ATADevice();

  void InterruptHandler();
  void Consume(ATARequest request);

  // uint16_t ata_base; // ata controller base address
  uint16_t bus_base_port;  // ata controller base address? same as ata_base?
  uint16_t ata_master;     // ata controller master address TODO what is this
  bool is_master;          // flag, master = 1, slave = 0
  uint8_t irq;             // irq # for this controller TODO how do i use this
  // proc::BlockedQueue* proc_queue;
  uint64_t num_sectors;
  // Queue<ATARequest> request_queue;
};

}  // namespace vfs

#endif  // VFS_ATA_H_

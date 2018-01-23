#ifndef KERNEL_PAGE_TABLE_H_
#define KERNEL_PAGE_TABLE_H_

#include "stdint.h"

class PageTable {
 public:
  PageTable(uint64_t p4_entry);
  ~PageTable();

  PageTable* Clone();

  uint64_t cr3();

  // Given a virtual address, returns the physical address it is mapped to.
  // Returns NULL_FRAME from frame.h if the requested address is not mapped.
  uint64_t GetPhysicalAddress(uint64_t virtual_address);

 private:
  uint64_t cr3_;
};

#endif  // KERNEL_PAGE_TABLE_H_

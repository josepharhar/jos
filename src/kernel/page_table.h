#ifndef KERNEL_PAGE_TABLE_H_
#define KERNEL_PAGE_TABLE_H_

#include "stdint.h"

namespace page {

uint64_t CopyPageTable(uint64_t cr3);

// Given a virtual address, returns the physical address it is mapped to.
// Returns NULL_FRAME from frame.h if the requested address is not mapped.
uint64_t GetPhysicalAddress(uint64_t cr3, uint64_t virtual_address);

}  // namespace page

#endif  // KERNEL_PAGE_TABLE_H_

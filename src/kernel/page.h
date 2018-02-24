#ifndef PAGE_H_
#define PAGE_H_

#include "stdint.h"

#define PAGE_SIZE_BYTES 4096
#define PAGES_PER_STACK 12

// indexes into p4_table
// each accounts for 512GB of virtual addresses
#define P4_IDENTITY_MAP 0
#define P4_KERNEL_HEAP 1
// 2 - 14 growth space
#define P4_KERNEL_STACKS 14
#define P4_USERSPACE_START 15

namespace page {

uint64_t CopyPageTable(uint64_t cr3);

enum GetPhysicalAddressRequestType {
  // DEMAND_ALLOCATION = 1,  // demand paging, address is valid
  FULL_ALLOCATION_KERNEL,  // actually back this address with phys
  FULL_ALLOCATION_USER,    // allocate with user access bit set
  NO_ALLOCATION,           // just get the phys addr if it exists
  GET_P4,                  // no allocation, just get this PageTableEntry
  GET_P3,                  // no allocation, just get this PageTableEntry
  GET_P2,                  // no allocation, just get this PageTableEntry
  GET_P1,                  // no allocation, just get this PageTableEntry
};

// Given a virtual address, returns the physical address it is mapped to.
// Returns NULL_FRAME from frame.h if the requested address is not mapped.
uint64_t GetPhysicalAddress(
    uint64_t cr3,
    uint64_t address,
    GetPhysicalAddressRequestType request = NO_ALLOCATION,
    bool debug = false);

void Init();

// these operate on the kernel heap
void* PageAllocate();
void* PageAllocateContiguous(int num_pages);
void PageFreeContiguous(void* page, int num_pages);
void PageFree(void* bottom_of_stack);

// these operate on kernel stacks
uint64_t StackAllocate();
void StackFree(uint64_t bottom_of_stack);

bool IsAddressInUserspace(uint64_t address);

void HandlePageFault(uint64_t error_code, uint64_t faulting_address);

int AllocateUserSpace(uint64_t address, uint64_t num_bytes);

}  // namespace page

#endif  // PAGE_H_

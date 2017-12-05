#ifndef PAGE_H_
#define PAGE_H_

#include "stdint.h"

#define PAGE_SIZE_BYTES 4096
#define STACK_SIZE_BYTES (4096 * 4096) // 2MB

// indexes into p4_table
// each accounts for 512GB of virtual addresses
#define P4_IDENTITY_MAP 0
#define P4_KERNEL_HEAP 1

#define P4_USERSPACE_TEXT 2

// 3 - 14 growth space
#define P4_KERNEL_STACKS 14
#define P4_USERSPACE_START 15

void PageInit();

// these operate on the kernel heap
void* PageAllocate();
void* PageAllocateContiguous(int num_pages);
void PageFreeContiguous(void* page, int num_pages);
void PageFree(void* page);

// these operate on kernel stacks
void* StackAllocate();
void StackFree(void* stack);

bool IsAddressInUserspace(uint64_t address);

void HandlePageFault(uint64_t error_code, uint64_t faulting_address);

int AllocateUserSpace(uint64_t address, uint64_t num_bytes);

uint64_t* Getcr3();
void Setcr3(uint64_t cr3);

#endif  // PAGE_H_

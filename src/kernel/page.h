#ifndef PAGE_H_
#define PAGE_H_

#include "stdint.h"

#define PAGE_SIZE_BYTES 4096
#define STACK_SIZE_BYTES (4096 * 4096) // 2MB

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

void* CopyCurrentPageTable();
void FreePageTable(void* page_table);

uint64_t* Getcr3();
void Setcr3(uint64_t cr3);

#endif  // PAGE_H_

#include "page_table.h"

#include "kmalloc.h"
#include "page.h"
#include "string.h"
#include "frame.h"
#include "printk.h"

namespace page {

// TODO combine this with the one from page.cc
struct VirtualAddress {
  uint64_t physical_frame_offset : 12;
  uint64_t p1_index : 9;  // index into p1 table
  uint64_t p2_index : 9;
  uint64_t p3_index : 9;
  uint64_t p4_index : 9;
  uint64_t sign_extended : 16;

  static VirtualAddress FromPointer(uint64_t address) {
    VirtualAddress* pointer = (VirtualAddress*)&address;
    return *pointer;
  }
} __attribute__((packed));

// TODO combine this with the one from page.cc
// http://os.phil-opp.com/entering-longmode.html#paging
struct PageTableEntry {
  uint64_t present : 1;
  uint64_t writable : 1;
  uint64_t user_accessible : 1;
  uint64_t write_through_cache : 1;
  uint64_t disable_cache : 1;
  uint64_t accessed : 1;
  uint64_t dirty : 1;
  uint64_t huge_page : 1;
  uint64_t global : 1;
  uint64_t available1 : 3;
  uint64_t address : 40;     // << 12 to get full address
  uint64_t available2 : 11;  // used for demand paging flags
  uint64_t no_execute : 1;

  uint64_t GetAddress() { return address << 12; }
  void SetAddress(void* pointer) { address = ((uint64_t)pointer) >> 12; }
} __attribute__((packed));
static struct PageTableEntry ToPageTableEntry(uint64_t entry) {
  struct PageTableEntry* pointer = (struct PageTableEntry*)&entry;
  return *pointer;
}
static uint64_t FromPageTableEntry(struct PageTableEntry table_entry) {
  uint64_t* pointer = (uint64_t*)&table_entry;
  return *pointer;
}
/*static uint64_t GetAddress(struct PageTableEntry* table_entry) {
  return table_entry->address << 12;
}
static void SetAddress(struct PageTableEntry* table_entry, void* pointer) {
  uint64_t address = (uint64_t)pointer;
  table_entry->address = address >> 12;
}*/
// flags for demand paging
// used in PageTableEntry.available2
#define PAGE_NOT_ALLOCATED 0  // pointer in this entry is not valid
#define PAGE_ALLOCATED 1      // pointer in this entry is valid

static void* CopyPageTableLevel(void* old_table_void, int level) {
  PageTableEntry* new_table = (PageTableEntry*)FrameAllocate();
  PageTableEntry* old_table = (PageTableEntry*)old_table_void;
  for (int i = 0; i < PAGE_SIZE_BYTES / sizeof(PageTableEntry); i++) {
    new_table[i] = old_table[i];

    if (level == 4 && i < P4_USERSPACE_START) {
      // don't copy inner stuff, its the same for the kernel
      continue;
    }

    if (!old_table[i].GetAddress()) {
      // this page table entry doesn't point to anything.
      continue;
    }

    if (level == 1) {
      // copy physical frame
      void* old_frame = (void*)old_table[i].GetAddress();
      void* new_frame = FrameAllocate();
      memcpy(new_frame, old_frame, PAGE_SIZE_BYTES);
      new_table[i].SetAddress(new_frame);
    } else {
      // copy page table level
      new_table[i].SetAddress(
          CopyPageTableLevel((void*)old_table[i].GetAddress(), level - 1));
    }
  }
  return new_table;
}

uint64_t CopyPageTable(uint64_t cr3) {
  return (uint64_t)CopyPageTableLevel((void*)cr3, 4);
}

void DeletePageTable(uint64_t cr3) {
  // TODO
}

// TODO this overlaps a lot of logic with GetP1Entry from page.cc
// TODO this should definitely be unit tested
uint64_t GetPhysicalAddress(uint64_t cr3, uint64_t address) {
  VirtualAddress virtual_address = VirtualAddress::FromPointer(address);

  PageTableEntry* p4_entry =
      (PageTableEntry*)(((uint64_t*)cr3) + virtual_address.p4_index);
  if (p4_entry->available2 != PAGE_ALLOCATED) {
    return NULL_FRAME;
  }

  PageTableEntry* p3_entry = (PageTableEntry*)p4_entry->GetAddress();
  if (p3_entry->available2 != PAGE_ALLOCATED) {
    return NULL_FRAME;
  }

  PageTableEntry* p2_entry = (PageTableEntry*)p3_entry->GetAddress();
  if (p2_entry->available2 != PAGE_ALLOCATED) {
    return NULL_FRAME;
  }

  PageTableEntry* p1_entry = (PageTableEntry*)p2_entry->GetAddress();
  if (p1_entry->available2 != PAGE_ALLOCATED) {
    return NULL_FRAME;
  }

  return p1_entry->GetAddress() + virtual_address.physical_frame_offset;
}

}  // namespace page

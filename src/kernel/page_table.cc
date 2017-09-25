#include "page_table.h"

#include "kmalloc.h"
#include "page.h"
#include "string.h"

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

PageTable::PageTable(uint64_t p4_entry) : p4_entry_(p4_entry) {}

// TODO free page table in destructor
PageTable::~PageTable() = default;

static void* CopyPageTableLevel(void* old_table_void, int level) {
  PageTableEntry* new_table = (PageTableEntry*)kmalloc(PAGE_SIZE_BYTES);
  PageTableEntry* old_table = (PageTableEntry*)old_table_void;
  for (int i = 0; i < PAGE_SIZE_BYTES / sizeof(PageTableEntry); i++) {
    new_table[i] = old_table[i];

    if (level == 4 && i < P4_USERSPACE_START) {
      // don't copy inner stuff, its the same for the kernel
    } else {
      uint64_t old_lower_level_pointer = new_table[i].GetAddress();
      if (old_lower_level_pointer) {
        if (level == 1) {
          // copy physical frame
          void* old_frame = (void*)old_lower_level_pointer;
          void* new_frame = kmalloc(PAGE_SIZE_BYTES);
          memcpy(new_frame, old_frame, PAGE_SIZE_BYTES);
          new_table[i].SetAddress(new_frame);
        } else {
          // copy page table level
          void* new_lower_level_pointer =
              CopyPageTableLevel((void*)old_lower_level_pointer, level - 1);
          new_table[i].SetAddress(new_lower_level_pointer);
        }
      }
    }
  }
  return new_table;
}

PageTable* PageTable::Clone() {
  return new PageTable((uint64_t)CopyPageTableLevel((void*)p4_entry_, 4));
}

uint64_t PageTable::p4_entry() {
  return p4_entry_;
}

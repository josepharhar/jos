#ifndef KERNEL_PAGE_TABLE_H_
#define KERNEL_PAGE_TABLE_H_

#include "stdint.h"

class PageTable {
 public:
  PageTable(uint64_t p4_entry);
  ~PageTable();

  PageTable* Clone();

  uint64_t p4_entry();

 private:
  uint64_t p4_entry_;  // value held in cr3 register, pointer to table
};

#endif  // KERNEL_PAGE_TABLE_H_

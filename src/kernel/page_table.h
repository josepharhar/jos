#ifndef KERNEL_PAGE_TABLE_H_
#define KERNEL_PAGE_TABLE_H_

#include "stdint.h"

class PageTable {
 public:
  PageTable();
  ~PageTable();

  uint64_t GetP1Entry();

  PageTable Clone();

 private:
  uint64_t p4_entry;
};

#endif  // KERNEL_PAGE_TABLE_H_

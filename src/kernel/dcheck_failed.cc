#include "dcheck.h"

#include "printk.h"

void DCHECKFailed(const char* condition) {
  printk("DCHECK failed: %s\n", condition);
  while (1) {
    asm volatile ("hlt");
  }
}

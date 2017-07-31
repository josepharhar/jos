#include "dcheck.h"

#include "printk.h"

void DCHECKFailed(const char* condition) {
  printk("DCHECK failed: %s\n", condition);

  while (1) {
    asm volatile ("hlt");
  }
}

void DCHECKFailedMessage(const char* condition, const char* format, ...) {
  printk("DCHECK failed: %s\n  ", condition);
  va_list list;
  va_start(list, format);
  vprintk(format, list);
  va_end(list);
  printk("\n");

  while (1) {
    asm volatile ("hlt");
  }
}

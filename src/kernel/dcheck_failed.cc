#include "dcheck.h"

#include "printk.h"
#include "vga.h"
#include "vprintf.h"

void DCHECKFailed(const char* condition) {
  printk("DCHECK failed: %s\n", condition);

  while (1) {
    asm volatile ("hlt");
  }
}

void DCHECKFailedMessage(const char* condition, const char* format, ...) {
  printk("DCHECK failed: %s\n  ", condition);

  // TODO why is this calling vprintk manually?
  va_list list;
  va_start(list, format);
  vprintf(format, list, VGA_display_char, VGA_display_str);
  va_end(list);
  printk("\n");

  while (1) {
    asm volatile ("hlt");
  }
}

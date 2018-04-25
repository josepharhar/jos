#include "dcheck.h"

#include "printk.h"
#include "vga.h"
#include "vprintf.h"

void DCHECKFailed(const char* condition) {
  printk("DCHECK failed: %s\n", condition);

  int one = 1;
  while (one);
}

void DCHECKFailedMessage(const char* condition, const char* format, ...) {
  printk("DCHECK failed: %s\n", condition);

  va_list list;
  va_start(list, format);
  vprintf(format, list, VGA_display_char, VGA_display_str);
  va_end(list);
  printk("\n");

  int one = 1;
  while (one);
}

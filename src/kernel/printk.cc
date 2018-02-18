#include "printk.h"

#include <stdarg.h>

#include "vga.h"
#include "irq.h"
#include "serial.h"
#include "vprintf.h"

//#define SERIAL_OUTPUT 1

int printk(const char* format, ...) {
  va_list list;
  va_start(list, format);

#ifdef SERIAL_OUTPUT
  CharPrinter char_printer = SerialWriteChar;
  StringPrinter string_printer = SerialWriteString;
#else
  CharPrinter char_printer = VGA_display_char;
  StringPrinter string_printer = VGA_display_str;
#endif
  int ret_value = vprintf(format, list, char_printer, string_printer);

  va_end(list);
  return ret_value;
}

void TestPrintk() {
  int hex = 0xABCD1234;
  printk("%%X 0xABCD1234: %X\n", hex);
  printk("%%x 0xabcd1234: %x\n", hex);

  printk("%%d -1234: %d\n", -1234);
  printk("%%u -1234: %u\n", (unsigned) -1234);

  //printk("\"asdf\": %s\nint -1234: %d\nunsigned 1234: %u\n", "asdf", -1234, (unsigned) 1234);
  printk("%%s \"asdf\": %s\n", "asdf");

  long unsigned long_unsigned = 0x11223344;
  long long unsigned long_long_unsigned = 0x1122334455667788;
  printk("%%lx: %lx\n%%llx: %llx\n", long_unsigned, long_long_unsigned);
  printk("%%qu: %qu\n", long_long_unsigned);

  printk("%%p: %p\n", &long_unsigned);

  printk("%%hd 1243: %hd\n", 1234);
  printk("%%hhd 'c': %hhd\n", 'c');
}

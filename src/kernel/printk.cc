#include "printk.h"

#include <stdarg.h>

#include "vga.h"
#include "irq.h"
#include "serial.h"

#define LOWER_CASE 0
#define UPPER_CASE 1

//#define SERIAL_OUTPUT 1

#ifdef SERIAL_OUTPUT
#define PRINT_STRING(string) SerialWriteString(string)
#define PRINT_CHAR(character) SerialWriteChar(character)
#else
#define PRINT_STRING(string) VGA_display_str(string)
#define PRINT_CHAR(character) VGA_display_char(character)
#endif

#define PRINT_NUMBER(type, base, upper_case) {        \
  {                                                   \
    auto value = va_arg(list, type);                  \
    if (value < 0) {                                  \
      PRINT_CHAR('-');                                \
    }                                                 \
    ToString(value, string_buffer, base, upper_case); \
    PRINT_STRING(string_buffer);                      \
  }                                                   \
}

template <typename T>
static char ToChar(T number, int upper_case) {
  if (number < 0) {
    number *= -1;
  }

  if (number > 15) {
    // error
    return '?';
  } else if (number < 10) {
    return ((char) number) + '0';
  } else if (upper_case) {
    return ((char) number) + 'A' - 10;
  } else {
    return ((char) number) + 'a' - 10;
  }
}

// doesn't include '-' for negative numbers
template <typename T>
static void ToString(T value, char* string, int base, int upper_case) {
  int num_digits = 0;
  if (!value) {
    num_digits = 1;
  } else {
    T value_copy = value;
    while (value_copy) {
      value_copy /= base;
      num_digits++;
    }
  }

  for (int i = 0; i < num_digits; i++) {
    char digit = ToChar(value % base, upper_case);
    value /= base;
    string[num_digits - 1 - i] = digit;
  }
  string[num_digits] = '\0';
}

int printk(const char* format, ...) {
  va_list list;
  va_start(list, format);
  int ret_value = vprintk(format, list);
  va_end(list);
  return ret_value;
}

// TODO what is this supposed to return?
int vprintk(const char* format, va_list list) {
  BEGIN_CS();

  //va_list list;
  //va_start(list, format);
  int int_value, num_digits;
  long int long_int_value;
  long long int long_long_int_value;
  char string_buffer[64] = {0};

  while (*format) {
    if (*format == '%') {
      format++;
      switch (*format) {
        case '%':
          PRINT_CHAR('%');
          break;

        case 'i':
        case 'd':
          PRINT_NUMBER(int, 10, LOWER_CASE);
          break;

        case 'u':
          PRINT_NUMBER(unsigned, 10, LOWER_CASE);
          break;

        case 'X':
          PRINT_NUMBER(unsigned, 16, UPPER_CASE);
          break;

        case 'x':
          PRINT_NUMBER(unsigned, 16, LOWER_CASE);
          break;

        case 'c':
          PRINT_CHAR((char) va_arg(list, int));
          break;

        case 'p':
          // pointers on x86_64 should be uint64_t
          // 0x11223344aabbccdd
          PRINT_STRING("0x");
          PRINT_NUMBER(long long unsigned, 16, UPPER_CASE);
          
          break;

        case 'h': // %h[h][dux]
          // h - half - int promoted from short
          // hh - half half - int promoted from char
          // TODO do you even have to do anything for these specifiers?
          format++;
          if (*format == 'h') {
            format++;
            switch (*format) {
              case 'i':
              case 'd':
                PRINT_NUMBER(int, 10, LOWER_CASE);
                break;
              case 'u':
                PRINT_NUMBER(unsigned, 10, LOWER_CASE);
                break;
              case 'x':
                PRINT_NUMBER(unsigned, 16, LOWER_CASE);
                break;
              case 'X':
                PRINT_NUMBER(unsigned, 16, UPPER_CASE);
                break;
            }
          } else {
            switch (*format) {
              case 'i':
              case 'd':
                PRINT_NUMBER(int, 10, LOWER_CASE);
                break;
              case 'u':
                PRINT_NUMBER(unsigned, 10, LOWER_CASE);
                break;
              case 'x':
                PRINT_NUMBER(unsigned, 16, LOWER_CASE);
                break;
              case 'X':
                PRINT_NUMBER(unsigned, 16, UPPER_CASE);
                break;
            }
          }
          break;

        case 'l': // %l[l][dux]
          // l - long - at least 32 bits
          // ll - long long - at least 64 bits
          format++;
          if (*format == 'l') {
            format++;
            switch (*format) {
              case 'i':
              case 'd':
                PRINT_NUMBER(long long int, 10, LOWER_CASE);
                break;
              case 'u':
                PRINT_NUMBER(long long unsigned, 10, LOWER_CASE);
                break;
              case 'x':
                PRINT_NUMBER(long long unsigned, 16, LOWER_CASE);
                break;
              case 'X':
                PRINT_NUMBER(long long unsigned, 16, UPPER_CASE);
                break;
            }
          } else {
            switch (*format) {
              case 'i':
              case 'd':
                PRINT_NUMBER(long int, 10, LOWER_CASE);
                break;
              case 'u':
                PRINT_NUMBER(long unsigned, 10, LOWER_CASE);
                break;
              case 'x':
                PRINT_NUMBER(long unsigned, 16, LOWER_CASE);
                break;
              case 'X':
                PRINT_NUMBER(long unsigned, 16, UPPER_CASE);
                break;
            }
          }
          break;

        case 'q': // %q[dux]
          // quad-word, 64 bits, same as "long long" or "ll"
          format++;
          switch (*format) {
            case 'i':
            case 'd':
              PRINT_NUMBER(long long int, 10, LOWER_CASE);
              break;
            case 'u':
              PRINT_NUMBER(long long unsigned, 10, LOWER_CASE);
              break;
            case 'x':
              PRINT_NUMBER(long long unsigned, 16, LOWER_CASE);
              break;
            case 'X':
              PRINT_NUMBER(long long unsigned, 16, UPPER_CASE);
              break;
          }
          break;

        case 's':
          PRINT_STRING(va_arg(list, const char*));
          break;

        default:
          // unrecognized format character
          PRINT_CHAR('?');
          break;
      }
    } else {
      PRINT_CHAR(*format);
    }

    format++;
  }

  //va_end(list);
  END_CS();
  return 0;
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

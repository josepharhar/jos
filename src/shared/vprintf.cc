#include "vprintf.h"

#define PRINT_NUMBER(type, format_state)                                     \
  {                                                                          \
    if (format_state.size_specifier == LONG) {                               \
      Print(va_arg(list, long type), char_printer, string_printer,           \
            format_state);                                                   \
    } else if (format_state.size_specifier == LONG_LONG) {                   \
      Print(va_arg(list, long long type), char_printer, string_printer,      \
            format_state);                                                   \
    } else {                                                                 \
      Print(va_arg(list, type), char_printer, string_printer, format_state); \
    }                                                                        \
  }

enum PaddingType {
  SPACE_PADDED,
  ZERO_PADDED,
};
enum CaseType {
  UPPER_CASE,
  LOWER_CASE,
};
enum SizeSpecifier {
  SIZE_UNSPECIFIED = 0,
  HALF,
  HALF_HALF,
  LONG,
  LONG_LONG,
};
struct FormatState {
  PaddingType padding_type;
  int padding_value;
  CaseType case_type;
  int base;
  SizeSpecifier size_specifier;
};

static bool IsNumber(char character) {
  return character >= '0' && character <= '9';
}

template <typename T>
static char ToChar(T number, FormatState format_state) {
  if (number < 0) {
    number *= -1;
  }

  if (number > 15) {
    // error
    return '?';
  } else if (number < 10) {
    return ((char)number) + '0';
  } else if (format_state.case_type == UPPER_CASE) {
    return ((char)number) + 'A' - 10;
  } else {
    return ((char)number) + 'a' - 10;
  }
}

// doesn't include '-' for negative numbers
template <typename T>
static void ToString(T value, char* string, FormatState format_state) {
  // calculate num_digits
  int num_digits = 0;
  if (!value) {
    num_digits = 1;
  } else {
    T value_copy = value;
    while (value_copy) {
      value_copy /= format_state.base;
      num_digits++;
    }
  }

  // add padding
  if (format_state.padding_value > num_digits) {
    char padding_char = ' ';
    if (format_state.padding_type == ZERO_PADDED) {
      padding_char = '0';
    }
    for (int i = 0; i < format_state.padding_value - num_digits; i++) {
      *string++ = padding_char;
    }
  }

  // add numbers
  for (int i = 0; i < num_digits; i++) {
    char digit = ToChar(value % format_state.base, format_state);
    value /= format_state.base;
    string[num_digits - 1 - i] = digit;
  }
  string[num_digits] = '\0';
}

template <typename T>
static void Print(T value,
                  CharPrinter char_printer,
                  StringPrinter string_printer,
                  FormatState format_state) {
  if (value < 0) {
    char_printer('-');
  }
  char string_buffer[512] = {0};
  ToString(value, string_buffer, format_state);
  string_printer(string_buffer);
}

static void Formatter(char** format,
                      va_list list,
                      CharPrinter char_printer,
                      StringPrinter string_printer,
                      FormatState format_state) {
  switch (**format) {
    // terminal formats

    case '%':
      char_printer('%');
      break;

    case 'i':
    case 'd':
      format_state.base = 10;
      PRINT_NUMBER(int, format_state);
      break;

    case 'u':
      format_state.base = 10;
      PRINT_NUMBER(unsigned, format_state);
      break;

    case 'X':
      format_state.base = 16;
      PRINT_NUMBER(unsigned, format_state);
      break;

    case 'x':
      format_state.base = 16;
      PRINT_NUMBER(unsigned, format_state);
      break;

    case 'c':
      char_printer((char)va_arg(list, int));
      break;

    case 'p':
      string_printer("0x");
      format_state.base = 16;
      format_state.padding_type = ZERO_PADDED;
      format_state.padding_value = 16;
      format_state.size_specifier = LONG_LONG;
      PRINT_NUMBER(unsigned, format_state);
      break;

    case 's':
      string_printer(va_arg(list, const char*));
      break;

    // non-terminal (recursive) formats

    case 'h':  // %h[h][dux]
      // h - half - int promoted from short
      // hh - half half - int promoted from char
      format_state.size_specifier = HALF;
      (*format)++;
      if (**format == 'h') {
        format_state.size_specifier = HALF_HALF;
        (*format)++;
      }
      Formatter(format, list, char_printer, string_printer, format_state);
      break;

    case 'l':  // %l[l][dux]
      // l - long - at least 32 bits
      // ll - long long - at least 64 bits
      format_state.size_specifier = LONG;
      (*format)++;
      if (**format == 'l') {
        format_state.size_specifier = LONG_LONG;
        (*format)++;
      }
      Formatter(format, list, char_printer, string_printer, format_state);
      break;

    case 'q':  // %q[dux]
      // quad-word, 64 bits, same as "long long" or "ll"
      (*format)++;
      format_state.size_specifier = LONG_LONG;
      Formatter(format, list, char_printer, string_printer, format_state);
      break;

    case '0':
      (*format)++;
      format_state.padding_type = ZERO_PADDED;
      Formatter(format, list, char_printer, string_printer, format_state);
      break;

    default:
      if (IsNumber(**format)) {
        // set padding value
        format_state.padding_value = 0;
        while (IsNumber(**format)) {
          format_state.padding_value *= 10;
          format_state.padding_value += (int)(**format - '0');
          (*format)++;
        }
        Formatter(format, list, char_printer, string_printer, format_state);
      } else {
        // unrecognized format character
        char_printer('?');
      }
      break;
  }
}

// TODO what is this supposed to return?
int vprintf(const char* format_const,
            va_list list,
            CharPrinter char_printer,
            StringPrinter string_printer) {
  // TODO write everything to a buffer and then just use a string printer
  // instead
  // of trying to use BEGIN_CS()/END_CS()
  // BEGIN_CS();

  // va_list list;
  // va_start(list, format);
  int int_value, num_digits;
  long int long_int_value;
  long long int long_long_int_value;
  char* format = (char*)format_const;

  while (*format) {
    if (*format == '%') {
      format++;
      FormatState format_state;
      format_state.padding_type = SPACE_PADDED;
      format_state.padding_value = 0;
      format_state.case_type = UPPER_CASE;
      format_state.base = 10;
      format_state.size_specifier = SIZE_UNSPECIFIED;
      Formatter(&format, list, char_printer, string_printer, format_state);
    } else {
      char_printer(*format);
    }
    format++;
  }

  // va_end(list);
  // END_CS();
  return 0;
}

#include "vga.h"

#include "string.h"
#include "stdint.h"
#include "irq.h"

#define VGA 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 20 // TODO is this 25?

static uint16_t* const VGA_BUFF = (uint16_t*) VGA;

static int cursor = 0;

static void scroll() {
  static uint16_t vga_buff_temp[VGA_WIDTH * VGA_HEIGHT] = {0};
  int vga_size_bytes = VGA_WIDTH * VGA_HEIGHT * sizeof(uint16_t);

  memcpy(vga_buff_temp, VGA_BUFF, vga_size_bytes);
  memcpy(VGA_BUFF, vga_buff_temp + VGA_WIDTH, vga_size_bytes - VGA_WIDTH * sizeof(uint16_t));
  memset(VGA_BUFF + (VGA_WIDTH * VGA_HEIGHT) - VGA_WIDTH, '\0', VGA_WIDTH * sizeof(uint16_t));

  /*void* destination = VGA_BUFF;
  void* source = VGA_BUFF + VGA_WIDTH;
  int num_bytes = (VGA_WIDTH * VGA_HEIGHT - VGA_WIDTH) * sizeof(uint16_t);
  memcpy(destination, source, num_bytes);
  memset(VGA_BUFF + VGA_WIDTH * VGA_HEIGHT - VGA_WIDTH, '\0', VGA_WIDTH);*/
}

static int line(int cursor) {
  return cursor - (cursor % VGA_WIDTH);
}

void VGA_clear() {
  BEGIN_CS();
  memset(VGA_BUFF, '\0', VGA_WIDTH * VGA_HEIGHT);
  END_CS();
}

void VGA_display_char(char display_char) {
  BEGIN_CS();

  if (display_char == '\n') {
    cursor = line(cursor);
    cursor += VGA_WIDTH;

    if (cursor >= VGA_WIDTH * VGA_HEIGHT) {
      scroll();
      cursor -= VGA_WIDTH;
    }
  } else if (display_char == '\r') {
    cursor = line(cursor);
  } else {
    VGA_BUFF[cursor] = (unsigned short) display_char;
    if ((cursor % VGA_WIDTH) < (VGA_WIDTH - 1)) {
      cursor++;
    }
  }

  END_CS();
}

void VGA_display_str(const char* str) {
  BEGIN_CS();
  while (*str) {
    VGA_display_char(*str);
    str++;
  }
  END_CS();
}

void VGA_display_attr_char(int x, int y, uint8_t c, uint16_t fg, uint16_t bg) {
  uint8_t attribute_byte = (bg << 4) | fg;
  uint16_t value = (attribute_byte << 8) | c;
  VGA_BUFF[y * VGA_WIDTH + x] = value;
}

int VGA_row_count() {
  return VGA_HEIGHT;
}

int VGA_col_count() {
  return VGA_WIDTH;
}

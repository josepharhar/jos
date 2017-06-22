#include "serial.h"

#include "asm.h"
#include "printk.h"
#include "vga.h"

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define PORT COM1

// https://www.lammertbies.nl/comm/info/serial-uart.html
#define RBR PORT // receiver buffer
#define THR PORT // transmitter holding
#define IER (PORT + 1) // interrupt enable
#define IIR (PORT + 2) // interrupt identification
#define LCR (PORT + 3) // line control
#define MCR (PORT + 4) // modem control
#define LSR (PORT + 5) // line status
#define MSR (PORT + 6) // modem status

// bellardo's example has size 16
//#define SERIAL_BUFFER_SIZE 4096
#define SERIAL_BUFFER_SIZE 256

/*static int serial_received() {
  return inb(LSR) & 1;
}
static char read_serial() {
  while (serial_received() == 0);
  return inb(PORT);
}
static void write_serial(char a) {
  while (is_transmit_empty() == 0);
  outb(PORT, a);
}*/

static int is_transmit_empty() {
  return inb(LSR) & 0x20;
}

static char serial_output_buffer[SERIAL_BUFFER_SIZE] = {0};
static int buffer_free_index = 0;
static int buffer_drain_index = 0;

void SerialInit() {
  outb(IER, 0x00);    // Disable all interrupts
  outb(LCR, 0x80);    // Enable DLAB (set baud rate divisor)
  outb(RBR, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  outb(IER, 0x00);    //                  (hi byte)
  outb(LCR, 0x03);    // 8 bits, no parity, one stop bit
  // dlab is now zero
  outb(IIR, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  outb(MCR, 0x0B);    // IRQs enabled, RTS/DSR set

  // after enabling interrupts here, we will immediately get the input available one
  outb(IER, 0xF); // enable all interrupts
  outb(IER, 1 << 1); // enable data ready to output interrupt only

  buffer_free_index = 0;
  buffer_drain_index = 0;
}

// https://users.csc.calpoly.edu/~bellardo/courses/2174/454/notes/CPE454-Week03-2.pdf
// head = in = free, tail = out = drain
// tail points to the next char to drain
//   or the head, in which case the buffer is empty
// head points to the next free spot in the buffer
//   or the tail, in which case the buffer is full
// if tail and head are the same, is the buffer empty or full???
//   if free moves onto drain, then its full
//   if drain moves onto free, its empty
//   the state of same is perfectly fine - initially

/// return 0 for consumed successfully, 1 for nothing to consume
static int TryConsume() {
  BEGIN_CS();
  int consume_failed;

  if (buffer_free_index == buffer_drain_index) {
    // buffer is empty, nothing to consume
  } else {
    if (is_transmit_empty()) {
      outb(PORT, serial_output_buffer[buffer_drain_index]);
      buffer_drain_index++;
      buffer_drain_index %= SERIAL_BUFFER_SIZE;
      consume_failed = 0;
    } else {
      consume_failed = 1;
    }
  }

  END_CS();
  return consume_failed;
}

// returns 0 on success, 1 on buffer full
static int ProducerAddChar(char input) {
  BEGIN_CS();

  int next_free_index = (buffer_free_index + 1) % SERIAL_BUFFER_SIZE;
  if (next_free_index == buffer_drain_index) {
    // buffer is full! reject this request
    VGA_display_str("SerialWrite() buffer is full, rejected request '");
    VGA_display_char(input);
    VGA_display_str("'\n");

    END_CS();
    return 1;
  } else {
    // add output and try to consume it immediately in case there is
    // no interrupt to trigger it
    serial_output_buffer[buffer_free_index] = input;
    buffer_free_index = next_free_index;
    TryConsume();

    END_CS();
    return 0;
  }
}

void SerialWriteChar(char character) {
  if (character == '\n') {
    // for serial i/o, we have to use carridge returns
    ProducerAddChar('\r');
  }
  ProducerAddChar(character);
}

void SerialWriteString(const char* string) {
  while (*string) {
    ProducerAddChar(*string);
    string++;
  }
}

void SerialWrite(const char* buff, int len) {
  for (int i = 0; i < len; i++) {
    if (ProducerAddChar(buff[i])) {
      // drop entire request if the buffer is full
      return;
    }
  }
}

void SerialHandleInterrupt() {
  // first figure out why the interrupt occured
  // check IIR (interrupt identification register) to get interrupt code
  uint8_t iir = inb(IIR);

  // bit 0 should always be 0 because there should be an interrupt
  // pending when an interrupt is triggered
  if (iir & 1) {
    VGA_display_str("IIR bit 0 was 1, expected 0 because there was an interrupt\n");
  }

  uint8_t interrupt_type_bits = (iir >> 1) & 0x7;
  char input;
  switch (interrupt_type_bits) {
    case 0:
      // modem status change, reset by MSR read
      printk("serial interrupt modem status change, MSR: 0x%X\n", inb(MSR));
      break;

    case 1:
      // transmitter holding register empty, reset by IIR read or THR write
      //printk("serial interrupt printing to screen ready\n");
      // LSR should always show that THR is empty here
      if (!(inb(LSR) & 0x20)) {
        VGA_display_str("[VGA] expected LSR to show data available, did not\n");
      } else {
        if (TryConsume()) {
          // nothing to consume. read IIR to reset interrupt?
          VGA_display_str("[VGA] nothing to consume (nothing printed to screen)\n");
          //inb(IIR);
        } else {
          // consume successfully
          VGA_display_str("[VGA] consumed successfully (should have printed to screen)\n");
        }
      }
      break;

    case 2:
      // received data available, reset by RBR read
      printk("serial interrupt received data available, RBR: 0x%X\n", inb(RBR));
      break;

    case 3:
      // line status change, reset by LSR read
      printk("serial interrupt line status change, LSR: 0x%X\n", inb(LSR));
      break;

    case 6:
      // character timeout (16550), reset by RBR read
      input = inb(RBR);
      printk("serial interrupt character timeout, RBR: 0x%X '%c'\n", input, input);
      break;

    default:
      // i think this should never happen
      printk("unknown serial interrupt: 0x%X\n", interrupt_type_bits);
      break;
  }
}

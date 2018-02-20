#include "keyboard.h"

#include "printk.h"
#include "proc.h"
#include "asm.h"
#include "irq.h"

#define PS2_DATA 0x60 // communication with a ps2 device within controller
#define PS2_CMD 0x64 // output to ps2 controller
#define PS2_STATUS PS2_CMD // input from ps2 controller
#define PS2_STATUS_OUTPUT 1 // must be 1 before reading from 0x60
#define PS2_STATUS_INPUT (1 << 1) // must be 0 before writing to 0x60 or 0x64

#define CMD_GET_CONFIGURATION 0x20
#define CMD_SET_CONFIGURATION 0x60
#define CMD_TEST_CONTROLLER 0xAA
#define CMD_DISABLE_PORT_1 0xAD
#define CMD_DISABLE_PORT_2 0xA7
#define CMD_ENABLE_PORT_1 0xAE
#define ECHO 0xEE

#define CONFIGURATION_FIRST_PORT_INTERRUPT 1
#define CONFIGURATION_SECOND_PORT_INTERRUPT (1 << 1)
#define CONFIGURATION_FIRST_PORT_TRANSLATION (1 << 6)

#define SCANCODE_RELEASED 0xF0
#define SCANCODE_LEFT_SHIFT 0x12
#define SCANCODE_RIGHT_SHIFT 0x59

#define KEYBOARD_BUFFER_SIZE 256

// same pattern as serial_output_buffer in serial.c
// TODO this should be a per-process buffer instead
static char keyboard_input_buffer[KEYBOARD_BUFFER_SIZE] = {0};
static int buffer_free_index = 0;
static int buffer_drain_index = 0;
static proc::BlockedQueue* proc_queue = 0;

static char scancode_map[0x100] = {0};
static char scancode_map_shift[0x100] = {0};
static uint8_t scancode_pressed[0x100] = {0};

static void InitializeScancodes() {
  scancode_map[0x0D] = '\t';
  scancode_map[0x0E] = '`';
  scancode_map[0x15] = 'q';
  scancode_map[0x16] = '1';
  scancode_map[0x1A] = 'z';
  scancode_map[0x1B] = 's';
  scancode_map[0x1C] = 'a';
  scancode_map[0x1D] = 'w';
  scancode_map[0x1E] = '2';
  scancode_map[0x21] = 'c';
  scancode_map[0x22] = 'x';
  scancode_map[0x23] = 'd';
  scancode_map[0x24] = 'e';
  scancode_map[0x25] = '4';
  scancode_map[0x26] = '3';
  scancode_map[0x29] = ' ';
  scancode_map[0x2A] = 'v';
  scancode_map[0x2B] = 'f';
  scancode_map[0x2C] = 't';
  scancode_map[0x2D] = 'r';
  scancode_map[0x2E] = '5';
  scancode_map[0x31] = 'n';
  scancode_map[0x32] = 'b';
  scancode_map[0x33] = 'h';
  scancode_map[0x34] = 'g';
  scancode_map[0x35] = 'y';
  scancode_map[0x36] = '6';
  scancode_map[0x3A] = 'm';
  scancode_map[0x3B] = 'j';
  scancode_map[0x3C] = 'u';
  scancode_map[0x3D] = '7';
  scancode_map[0x3E] = '8';
  scancode_map[0x41] = ',';
  scancode_map[0x42] = 'k';
  scancode_map[0x43] = 'i';
  scancode_map[0x44] = 'o';
  scancode_map[0x45] = '0';
  scancode_map[0x46] = '9';
  scancode_map[0x49] = '.';
  scancode_map[0x4A] = '/';
  scancode_map[0x4B] = 'l';
  scancode_map[0x4C] = ';';
  scancode_map[0x4D] = 'p';
  scancode_map[0x4E] = '-';
  scancode_map[0x52] = '\'';
  scancode_map[0x54] = '[';
  scancode_map[0x55] = '=';
  scancode_map[0x5A] = '\n';
  scancode_map[0x5B] = ']';
  scancode_map[0x5D] = '\\';

  scancode_map_shift[0x0D] = '\t';
  scancode_map_shift[0x0E] = '~';
  scancode_map_shift[0x15] = 'Q';
  scancode_map_shift[0x16] = '!';
  scancode_map_shift[0x1A] = 'Z';
  scancode_map_shift[0x1B] = 'S';
  scancode_map_shift[0x1C] = 'A';
  scancode_map_shift[0x1D] = 'W';
  scancode_map_shift[0x1E] = '@';
  scancode_map_shift[0x21] = 'C';
  scancode_map_shift[0x22] = 'X';
  scancode_map_shift[0x23] = 'D';
  scancode_map_shift[0x24] = 'E';
  scancode_map_shift[0x25] = '$';
  scancode_map_shift[0x26] = '#';
  scancode_map_shift[0x29] = ' ';
  scancode_map_shift[0x2A] = 'V';
  scancode_map_shift[0x2B] = 'F';
  scancode_map_shift[0x2C] = 'T';
  scancode_map_shift[0x2D] = 'R';
  scancode_map_shift[0x2E] = '%';
  scancode_map_shift[0x31] = 'N';
  scancode_map_shift[0x32] = 'B';
  scancode_map_shift[0x33] = 'H';
  scancode_map_shift[0x34] = 'G';
  scancode_map_shift[0x35] = 'Y';
  scancode_map_shift[0x36] = '^';
  scancode_map_shift[0x3A] = 'M';
  scancode_map_shift[0x3B] = 'J';
  scancode_map_shift[0x3C] = 'U';
  scancode_map_shift[0x3D] = '&';
  scancode_map_shift[0x3E] = '*';
  scancode_map_shift[0x41] = '<';
  scancode_map_shift[0x42] = 'K';
  scancode_map_shift[0x43] = 'I';
  scancode_map_shift[0x44] = 'O';
  scancode_map_shift[0x45] = ')';
  scancode_map_shift[0x46] = '(';
  scancode_map_shift[0x49] = '>';
  scancode_map_shift[0x4A] = '?';
  scancode_map_shift[0x4B] = 'L';
  scancode_map_shift[0x4C] = ':';
  scancode_map_shift[0x4D] = 'P';
  scancode_map_shift[0x4E] = '_';
  scancode_map_shift[0x52] = '"';
  scancode_map_shift[0x54] = '{';
  scancode_map_shift[0x55] = '+';
  scancode_map_shift[0x5A] = '\n';
  scancode_map_shift[0x5B] = '}';
  scancode_map_shift[0x5D] = '|';
}

/*char GetScancode() {
  char status = 0;
  do {
    if (inb(0x60) != status) {
      status = inb(0x60);
      if (status > 0) {
        return status
      }
    }
  } while(1);
}

char GetChar() {
  return scancode[getScancode() + 1];
}*/

static void SendChar(uint8_t value) {
  outb(PS2_CMD, value);
}

static uint8_t GetChar() {
  uint8_t status = inb(PS2_STATUS);
  while (!(status & PS2_STATUS_OUTPUT)) {
    status = inb(PS2_STATUS);
  }
  return inb(PS2_DATA);
}

static void SendCharToData(uint8_t data) {
  uint8_t status = inb(PS2_STATUS);
  while (status & PS2_STATUS_INPUT) {
    status = inb(PS2_STATUS);
  }
  outb(PS2_DATA, data);
}

static uint8_t GetCharAsync() {
  if (!(inb(PS2_STATUS) & PS2_STATUS_OUTPUT)) {
    // when there isn't output available, return zero now
    return '\0';
  }
  return inb(PS2_DATA);
}

char PollKeyboard() {
  uint8_t scancode = GetCharAsync();

  if (scancode == SCANCODE_RELEASED) {
    uint8_t released_scancode = GetCharAsync();
    scancode_pressed[released_scancode] = 0;
    return '\0';
    // TODO should anything else happen when you release a key?
  }

  scancode_pressed[scancode] = 1;
  
  if (scancode_map[scancode]) {
    if (scancode_pressed[SCANCODE_LEFT_SHIFT] || scancode_pressed[SCANCODE_RIGHT_SHIFT]) {
      return scancode_map_shift[scancode];
    }
    return scancode_map[scancode];
  }

  // unmapped/unprintable scancode
  return '\0';
}

// KBD_read code below
//
// an interrupt is triggered when input is ready to be put into the buffer
// KeyboardRead() reads from the buffer, or blocks if the buffer is empty

// interrupts are disabled
static void HandleKeyboardInterrupt(uint64_t interrupt_number, void* arg) {
  char keyboard_input = PollKeyboard();

  if (keyboard_input) {
    // producer - add char to keyboard_input_buffer
    int next_free_index = (buffer_free_index + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_free_index == buffer_drain_index) {
      // buffer is full! reject this request
      printk("keyboard input buffer full, input: '%c'\n", keyboard_input);

    } else {
      // add output and try to consume it immediately
      keyboard_input_buffer[buffer_free_index] = keyboard_input;
      buffer_free_index = next_free_index;

      // consume - unblock a process
      printk("KeyboardRead() calling proc_queue->UnblockHead(): %d\n", proc_queue->UnblockHead());
    }
  }
}

// blocking keyboard character reader
char KeyboardRead() {
  if (!proc::IsRunning()) {
    printk("KeyboardRead() called without current_proc\n");
    return '\0';
  }

  // TODO shouldn't we try to consume before blocking?
  //      if there is already input in the buffer we shouldn't block
  printk("KeyboardRead() calling proc_queue->BlockCurrentProc()...\n");
  proc_queue->BlockCurrentProc();
  printk("KeyboardRead() returned from proc_queue->BlockCurrentProc()\n");

  if (buffer_free_index == buffer_drain_index) {
    printk("KeyboardRead() proc unblocked with empty buffer!\nthis should never happen!\n");
    return '\0';
  }

  char keyboard_input = keyboard_input_buffer[buffer_drain_index];
  buffer_drain_index++;
  buffer_drain_index %= KEYBOARD_BUFFER_SIZE;
  return keyboard_input;
}


void KeyboardInit() {
  IRQSetHandler(IRQHandlerEmpty, PIC1_OFFSET + 1, 0); // block interrupts during configuration

  InitializeScancodes();

  // disable the PS2 controller
  SendChar(CMD_DISABLE_PORT_1);
  SendChar(CMD_DISABLE_PORT_2);

  // flush the output buffer
  inb(PS2_STATUS);
  
  // get configuration byte
  SendChar(CMD_GET_CONFIGURATION);
  uint8_t configuration = GetChar();
  
  // change configuration byte
  configuration &= ~CONFIGURATION_FIRST_PORT_TRANSLATION;
  //configuration &= ~CONFIGURATION_FIRST_PORT_INTERRUPT;
  configuration |= CONFIGURATION_FIRST_PORT_INTERRUPT;
  configuration &= ~CONFIGURATION_SECOND_PORT_INTERRUPT;
  //configuration |= CONFIGURATION_SECOND_PORT_INTERRUPT;

  // set configuration byte
  SendChar(CMD_SET_CONFIGURATION);
  SendCharToData(configuration);

  // controller self test
  SendChar(CMD_TEST_CONTROLLER);
  uint8_t test_response = GetChar();
  if (test_response == 0x55) {
    // test passed
  } else if (test_response == 0xFC) {
    // test failed
    printk("ps2 controller test failed\n");
  } else {
    // unknown response
    printk("unknown ps2 controller test response: 0x%X\n", test_response);
  }

  // enable first port
  SendChar(CMD_ENABLE_PORT_1);

  // keyboard echo test
  SendCharToData(ECHO);
  uint8_t echo_response = GetChar();
  if (echo_response == ECHO) {
    // echo test passed, do nothing
  } else {
    // echo failed
    printk("echo test failed. got 0x%X\n", echo_response);
  }

  buffer_free_index = 0;
  buffer_drain_index = 0;
  proc_queue = new proc::BlockedQueue();

  IRQSetHandler(&HandleKeyboardInterrupt, PIC1_OFFSET + 1, 0);
}

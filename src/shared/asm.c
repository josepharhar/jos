#include "asm.h"

void outb(uint16_t port, uint8_t val) {
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
  /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
   * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
   * The  outb  %al, %dx  encoding is the only option for all other cases.
   * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}
void outw(uint16_t port, uint16_t val) {
  asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
uint16_t inw(uint16_t port) {
  uint16_t ret;
  asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void io_wait() {
  asm volatile ("jmp 1f\n\t" "1:jmp 2f\n\t" "2:");
  // asm volatile ("outb %%al, $0x80" : : "a"(0));
}

unsigned long get_flags() {
  unsigned long flags;
  asm volatile ("pushf\n\t" "pop %0" : "=g"(flags));
  return flags;
}

int are_interrupts_enabled() {
  unsigned long flags;
  asm volatile ("pushf\n\t" "pop %0" : "=g"(flags));
  return flags & (1 << 9);
}

void lidt(void* base, uint16_t size) {
  struct {
    uint16_t length;
    void* base;
  } __attribute__((packed)) IDTR = { size, base };
  asm ("lidt %0" : : "m"(IDTR));
}

/*void cpuid(int code, uint32_t* a, uint32_t d) {
  asm volatile ("cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx");
}*/

uint64_t rdtsc() {
  uint64_t ret;
  asm volatile ("rdtsc" : "=A"(ret));
  return ret;
}

unsigned long read_cr0() {
  unsigned long val;
  asm volatile ("mov %%cr0, %0" : "=r"(val));
  return val;
}

void invlpg(void* m) {
  /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
  asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

void wrmsr(uint32_t msr_id, uint64_t msr_value) {
  asm volatile ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
}

uint64_t rdmsr(uint32_t msr_id) {
  uint64_t msr_value;
  asm volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
  return msr_value;
}

void cli() {
  asm volatile ("cli");
}

void sti() {
  asm volatile ("sti");
}

void hlt() {
  if (!are_interrupts_enabled()) {
    //printk("hlt() called with interrupts disabled!\n");
    sti();
    while (1) {
      asm volatile ("hlt");
    }
  }

  ///printk("are_interrupts_enabled(): %d\n", are_interrupts_enabled());
  asm volatile ("hlt");
}

uint64_t Getcr3() {
  uint64_t cr3_value;
  GET_REGISTER("cr3", cr3_value);
  return cr3_value;
}

void Setcr3(uint64_t new_cr3) {
  uint64_t cr3_value = new_cr3;
  SET_REGISTER("cr3", cr3_value);
}

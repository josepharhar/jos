#ifndef ASM_H_
#define ASM_H_

#include "stdint.h"

// disable interrupts if they are enabled
#define BEGIN_CS()                                             \
  int __interrupts_were_enabled = are_interrupts_enabled();    \
  if (__interrupts_were_enabled) {                             \
    cli();                                                     \
  }

// re-enable interrupts if they were enabled before
#define END_CS()                                          \
  if (__interrupts_were_enabled) {                        \
    sti();                                                \
  }

// SET_REGISTER("rax", uint64_variable);
#define SET_REGISTER(reg, variable) \
  asm volatile ("movq %0, %%" reg : : "r"(variable));

// GET_REGISTER("rax", uint64_variable);
#define GET_REGISTER(reg, variable) \
  asm volatile ("movq %%" reg ", %0" : "=r"(variable) : );

#define HALT_LOOP() while (1) { asm volatile ("hlt"); }

#ifdef __cplusplus
extern "C" {
#endif

void cli(); // disables interrupts
void sti(); // enables interrupts
unsigned long get_flags();

void hlt();

// http://wiki.osdev.org/Inline_Assembly/Examples

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void io_wait();

int are_interrupts_enabled();
void lidt(void* base, uint16_t size);

//void cpuid(int code, uint32_t* a, uint32_t d);
uint64_t rdtsc();
unsigned long read_cr0();
void invlpg(void* m);
void wrmsr(uint32_t msr_id, uint64_t msr_value);
uint64_t rdmsr(uint32_t msr_id);

uint64_t Getcr3();
void Setcr3(uint64_t new_cr3);

#ifdef __cplusplus
}
#endif

#endif  // ASM_H_

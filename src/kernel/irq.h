#ifndef IRQ_H_
#define IRQ_H_

#include "asm.h"
#include "stdint.h"

// global for keyboard to set interrupt handler based on pic offset
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

typedef void (*IRQHandler)(uint64_t interrupt_number, void* arg);

#ifdef __cplusplus
extern "C" {
#endif

void IRQInit();
void IRQSetMask(uint8_t irq);
void IRQClearMask(uint8_t irq);
int IRQGetMask();
void IRQEndOfInterrupt(int irq);
void IRQSetHandler(IRQHandler handler, uint64_t interrupt_number, void* arg);

void c_interrupt_handler(uint64_t interrupt_number);
void c_interrupt_handler_2param(uint64_t interrupt_number, uint64_t error_code);

void IRQHandlerEmpty(uint64_t interrupt_number, void* arg);
void IRQHandlerDefault(uint64_t interrupt_number, void* arg);

#ifdef __cplusplus
}
#endif

#endif  // IRQ_H_

#include "pit.h"

#include "irq.h"
#include "printk.h"

static uint64_t time = 0;

static void HandlePitInterrupt(uint64_t interrupt_number, void* arg) {
  //printk("Programmable Interrupt Timer interrupt\n");
  time++;

  // TODO make this switch processes
  // TODO verify this doesn't interrupt any other IRQ handlers.
  //   already verified it doesn't interrupt keyboard handling IRQ.
}

void PitInit() {
  IRQSetHandler(HandlePitInterrupt, PIC1_OFFSET, 0 /* arg */);

  IRQClearMask(0);  // enables PIT interrupts
}

#include "pit.h"

#include "irq.h"
#include "printk.h"
#include "proc.h"

static uint64_t time = 0;

static void HandlePitInterrupt(uint64_t interrupt_number, void* arg) {
  // printk("Programmable Interrupt Timer interrupt\n");
  time++;

  // preemptive multitasking
  proc::PreemptProc();
}

void PitInit() {
  IRQSetHandler(HandlePitInterrupt, PIC1_OFFSET, 0 /* arg */);

  IRQClearMask(0);  // enables PIT interrupts
}

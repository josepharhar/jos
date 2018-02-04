#include "kernel/time.h"
#include "stdint.h"
#include "printk.h"

namespace time {

static uint64_t time = 0;

void TimerInterrupt() {
  time++;
  // TODO store time elapsed in ms for proc use
  // TODO switch procs
}

}  // namespace time

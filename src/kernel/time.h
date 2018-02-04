#ifndef KERNEL_TIME_H_
#define KERNEL_TIME_H_

namespace time {

// Called once for each PIC timer interrupt
void TimerInterrupt();

}  // namespace time

#endif  // KERNEL_TIME_H_

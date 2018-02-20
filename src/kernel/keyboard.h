#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "stdint.h"
#include "getc.h"

void KeyboardInit();

// returns '\0' if the keyboard input is not for printing
char PollKeyboard();

// must be called from a process
// blocks the process until keyboard input is available
char KeyboardRead();

void KeyboardReadNoNesting(SyscallGetcParams* params);

#endif  // KEYBOARD_H_

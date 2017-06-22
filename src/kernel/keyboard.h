#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "stdint.h"

void KeyboardInit();

// returns '\0' if the keyboard input is not for printing
char PollKeyboard();

// must be called from a process
// blocks the process until keyboard input is available
char KeyboardRead();

#endif  // KEYBOARD_H_

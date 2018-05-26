#include "stdlib.h"

static int pow_helper(int base, int power) {
  if (power == 0) {
    return 1;
  }
  return base * pow_helper(base, power - 1);
}

int atoi(char* string) {
  bool negative = false;
  if (*string == '-') {
    negative = true;
    string++;
  }

  int string_length = 0;
  while (string[string_length] >= '0' && string[string_length] <= '9') {
    string_length++;
  }

  int total = 0;
  for (int i = 0; i < string_length; i++) {
    int current_multiplier = pow_helper(10, string_length - i - 1);
    int this_value = (int)(string[i] - '0');
    total += this_value * current_multiplier;
  }

  if (negative) {
    total *= -1;
  }
  return total;
}

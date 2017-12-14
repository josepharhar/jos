#include "jstring.h"

namespace stdj {

string::string(char* other_string) {
  while (*other_string) {
    Add(*other_string++);
  }
}

}  // namespace stdj

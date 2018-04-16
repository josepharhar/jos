#include "jstring.h"

namespace stdj {

string::string() {}

string::string(const char* other_string) {
  CopyFrom(other_string);
}

string::string(char* other_string) {
  CopyFrom(other_string);
}

string::string(Array<char> array) {
  for (int i = 0; i < array.Size(); i++) {
    Add(array[i]);
  }
}

const char* string::c_str() {
  return Data();
}

void string::CopyFrom(const char* other_string) {
  while (*other_string) {
    Add(*other_string++);
  }
}

}  // namespace stdj

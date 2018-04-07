#include "jstring.h"

namespace stdj {

string::string() {}

string::string(const string& other) : stdj::Array<char>(other) {}

string& string::operator=(const string& other) {
  stdj::Array<char>::operator=(other);
  return *this;
}

string::string(const char* other_string) {
  while (*other_string) {
    Add(*other_string++);
  }
}

string::string(Array<char> array) {
  for (int i = 0; i < array.Size(); i++) {
    Add(array[i]);
  }
}

const char* string::c_str() {
  return Data();
}

}  // namespace stdj

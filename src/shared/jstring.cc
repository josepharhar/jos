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

stdj::Array<stdj::string> string::Split(stdj::string delimiter) {
  stdj::Array<stdj::string> output;

  int last_delimiter_index = 0;
  int i = 0;
  while (i < Size() + 1 - delimiter.Size()) {
    stdj::string possible_delimiter = Substring(i, i + delimiter.Size());
    if (delimiter == possible_delimiter) {
      if (last_delimiter_index != i) {
        stdj::string part = Substring(last_delimiter_index, i);
        output.Add(part);
      }
      i += delimiter.Size();
      last_delimiter_index = i;
    } else {
      i++;
    }
  }

  if (last_delimiter_index != i) {
    stdj::string last_part = Substring(last_delimiter_index, i);
    output.Add(last_part);
  }

  return output;
}

}  // namespace stdj

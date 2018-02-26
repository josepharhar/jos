#include "vfs2.h"

namespace vfs {

Filepath::Filepath() {}

Filepath::Filepath(stdj::string string) {
  int last_slash_index = -1;
  int i;
  for (i = 0; i < string.Size(); i++) {
    if (string[i] == '/') {
      // check for parsing multiple slashes in a row, ignore them
      if (last_slash_index < i - 1) {
        strings_.Add(string.Substring(last_slash_index + 1, i));
      }
      last_slash_index = i;
    }
  }

  // add the remainder
  if (last_slash_index != i - 1) {
    strings_.Add(string.Substring(last_slash_index + 1, i));
  }
}

Filepath::~Filepath() {}

void Filepath::Append(stdj::string string) {
  strings_.Add(string);
}

void Filepath::Append(Filepath other) {
  for (int i = 0; i < other.strings_.Size(); i++) {
    strings_.Add(other.strings_[i]);
  }
}

stdj::Array<stdj::string> Filepath::GetArray() {
  return strings_;
}

}  // namespace vfs

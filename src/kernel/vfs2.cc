#include "vfs2.h"

namespace vfs {

Filepath::Filepath() {}

Filepath::Filepath(const char* string) {
  // TODO
}

Filepath::~Filepath() {}

void Filepath::Append(stdj::string string) {
  strings.Add(string);
}

}  // namespace vfs

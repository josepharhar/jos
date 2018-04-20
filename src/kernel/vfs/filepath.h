#ifndef VFS_FILEPATH_
#define VFS_FILEPATH_

#include "shared/jstring.h"

namespace vfs {

class Filepath {
 public:
  Filepath();
  Filepath(stdj::string string);

  ~Filepath();

  Filepath(const Filepath& other);
  Filepath& operator=(const Filepath& other);

  void Append(stdj::string string);
  void Append(Filepath other);  // TODO make constref
  stdj::Array<stdj::string> GetArray();

  int Size();
  stdj::string RemoveFirst();
  stdj::string RemoveLast();

  stdj::string ToString();

 private:
  stdj::Array<stdj::string> strings_;
};

}  // namespace vfs

#endif  // VFS_FILEPATH_

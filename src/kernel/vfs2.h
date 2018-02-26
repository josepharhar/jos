#ifndef KERNEL_VFS2_H_
#define KERNEL_VFS2_H_

#include "shared/jarray.h"
#include "shared/jstring.h"

namespace vfs {

class Filepath {
 public:
  Filepath();
  Filepath(stdj::string string);

  ~Filepath();

  Filepath(const Filepath& other) = delete;
  Filepath& operator=(const Filepath& other) = delete;

  Filepath(Filepath&& other) = delete;
  Filepath& operator=(Filepath&& other) = delete;

  void Append(stdj::string string);
  void Append(Filepath other); // TODO make constref
  stdj::Array<stdj::string> GetArray();

 private:
  stdj::Array<stdj::string> strings_;
};

}  // namespace vfs

#endif  // KERNEL_VFS2_H_

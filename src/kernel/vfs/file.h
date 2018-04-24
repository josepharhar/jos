#ifndef VFS_FILE_H_
#define VFS_FILE_H_

#include "kernel/vfs/vfs2.h"

namespace vfs {

class Inode;
class Superblock;

class File {
 public:
  File(Inode* inode);

  typedef void (*FileRdWrCallback)(void*);
  int Read(void* dest,
           uint64_t length,
           FileRdWrCallback callback,
           void* callback_arg);
  int Write(void* src,
            uint64_t length,
            FileRdWrCallback callback,
            void* callback_arg);
  int Seek(uint64_t offset);
  int Close();
  // ? static int Close(File** file);

  uint64_t GetSize();
  uint64_t GetOffset();

 private:
  Inode* inode_;
  uint64_t offset_;

  static void ReadReadCluster(void* arg);
};

}  // namespace vfs

#endif  // VFS_FILE_H_

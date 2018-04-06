#ifndef VFS_FILE_H_
#define VFS_FILE_H_

#include "kernel/vfs/vfs2.h"

namespace vfs {

class Inode;
class Superblock;

class File {
 public:
  File(Inode* inode);

  int Read(uint8_t* dest, uint64_t length);
  int Write(uint8_t* src, uint64_t length);
  int Seek(uint64_t offset);
  int Close();
  // ? static int Close(File** file);

  // todo eh
  uint64_t GetSize();

 private:
  Inode* inode;
  uint64_t offset;
};

}  // namespace vfs

#endif  // VFS_FILE_H_

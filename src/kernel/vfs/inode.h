#ifndef VFS_INODE_H_
#define VFS_INODE_H_

#include "shared/jarray.h"

#include "kernel/vfs/vfs2.h"

namespace vfs {

class Inode {
 public:
  Inode(uint64_t cluster,
        char* name,
        bool is_directory,
        Superblock* superblock);

  static void Destroy(Inode* inode);

  File* Open();
  typedef void (*ReadDirCallback)(stdj::Array<Inode*>);
  void ReadDir(ReadDirCallback callback);
  bool IsDirectory();
  char* GetName();

  uint64_t GetSize();
  uint64_t GetCluster();
  Superblock* GetSuperblock();

  // TODO
  // int Unlink(const char* name); // delete files in dir?

  // TODO make this private and move to constructor
  // FAT32 specific information
  uint64_t cluster;  // points to first cluster of this file
  char filename[LFN_BUFFER_LENGTH];
  uint64_t size;

 private:
  Superblock* superblock;
  bool is_directory;
  // TODO
  /*struct timeval mod_time, acc_time, create_time;
  mode_t st_mode;
  uid_t uid;
  gid_t gid;
  uint64_t ino_num;*/
};

}  // namespace vfs

#endif  // VFS_INODE_H_

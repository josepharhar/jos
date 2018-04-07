#ifndef VFS_FIND_FILE_H_
#define VFS_FIND_FILE_H_

#include "kernel/vfs/inode.h"
#include "kernel/vfs/filepath.h"

namespace vfs {

typedef void (*FindFileCallback)(Inode* inode);
void FindFile(Inode* root_inode, Filepath filepath, FindFileCallback callback);

}  // namespace vfs

#endif  // VFS_FIND_FILE_H_

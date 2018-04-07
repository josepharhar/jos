#ifndef VFS_FIND_FILE_H_
#define VFS_FIND_FILE_H_

#include "kernel/vfs/inode.h"
#include "kernel/vfs/filepath.h"

namespace vfs {

typedef void (*FindFileCallback)(Inode*, void*);
void FindFile(Inode* root_inode,
              Filepath filepath,
              FindFileCallback callback,
              void* callback_arg);

}  // namespace vfs

#endif  // VFS_FIND_FILE_H_

#ifndef VFS_FIND_FILE_H_
#define VFS_FIND_FILE_H_

namespace vfs {

typedef void (*FindFileCallback)(Inode* inode);
void FindFile(Inode* inode, Filepath filepath, FileFileCallback callback);

}  // namespace vfs

#endif  // VFS_FIND_FILE_H_

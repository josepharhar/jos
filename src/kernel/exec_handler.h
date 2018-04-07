#ifndef EXEC_HANDLER_H_
#define EXEC_HANDLER_H_

#include "kernel/vfs/inode.h"

void InitExec(vfs::Inode* root_directory);

#endif  // EXEC_HANDLER_H_

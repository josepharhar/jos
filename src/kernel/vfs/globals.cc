#include "globals.h"

namespace vfs {

static Inode* root_directory = 0;

Inode* GetRootDirectory() {
  return root_directory;
}

void SetRootDirectory(Inode* new_root_directory) {
  root_directory = new_root_directory;
}

}  // namespace vfs

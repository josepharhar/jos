#include "files.h"

#include "string.h"
#include "ref_counted.h"

// disregards paths, directories, just finds the name
Inode FindFile(Inode inode, char* filename) {
  if (inode.IsDirectory()) {
    stdj::Array<Inode> sub_inodes = inode.ReadDir();

    for (int i = 0; i < sub_inodes.Size(); i++) {
      Inode sub_inode = sub_inodes[i];
      if (strcmp(sub_inode.GetName(), "..") && strcmp(sub_inode.GetName(), ".")) {
        Inode sub_result = FindFile(sub_inode, filename);
        if (sub_result) {
          return sub_result;
        }
      }
    }
    return 0;
  } else {
    if (!strcmp(inode.GetName(), filename)) {
      return inode;
    } else {
      return 0;
    }
  }
}

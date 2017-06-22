#include "files.h"

#include "string.h"

// disregards paths, directories, just finds the name
Inode* FindFile(Inode* inode, char* filename) {
  if (inode->IsDirectory()) {
    List<Inode> sub_inodes = inode->ReadDir();
    Inode* sub_inode = sub_inodes.GetHead();
    while (sub_inode) {
      if (strcmp(sub_inode->GetName(), "..") && strcmp(sub_inode->GetName(), ".")) {
        Inode* sub_result = FindFile(sub_inode, filename);
        if (sub_result) {
          return sub_result;
        }
      }

      Inode* sub_inode_old = sub_inode;
      sub_inode = sub_inodes.GetNextNoLoop(sub_inode);
      kfree(sub_inode_old);
    }
    return 0;
  } else {
    if (!strcmp(inode->GetName(), filename)) {
      return inode;
    } else {
      return 0;
    }
  }
}

#include "files.h"

#include "string.h"
#include "ref_counted.h"

// disregards paths, directories, just finds the name
Inode* FindFile(Inode* inode, char* filename) {
  if (inode->IsDirectory()) {
    LinkedList<Inode*> sub_inodes = inode->ReadDir();

    RefCounted<Iterator<Inode*>> iterator(sub_inodes.GetIterator());
    while (iterator->HasNext()) {
      Inode* sub_inode = iterator->Next();
      if (strcmp(sub_inode->GetName(), "..") && strcmp(sub_inode->GetName(), ".")) {
        Inode* sub_result = FindFile(sub_inode, filename);
        if (sub_result) {
          return sub_result;
        }
      }

      delete sub_inode;
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

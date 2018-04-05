#include "files.h"

#include "string.h"
#include "ref_counted.h"
#include "printk.h"

// disregards paths, directories, just finds the name
Inode* FindFile(Inode* inode, char* filename) {
  printk("11\n");
  if (inode->IsDirectory()) {
    printk("12\n");
    LinkedList<Inode*>* sub_inodes = inode->ReadDir();
    printk("13\n");

    //RefCounted<Iterator<Inode*>> iterator(sub_inodes->GetIterator());
    Iterator<Inode*>* iterator = sub_inodes->GetIterator();
    printk("14\n");
    while (iterator->HasNext()) {
      printk("15\n");
      Inode* sub_inode = iterator->Next();
      printk("16\n");
      if (strcmp(sub_inode->GetName(), "..") && strcmp(sub_inode->GetName(), ".")) {
        printk("17\n");
        Inode* sub_result = FindFile(sub_inode, filename);
        printk("18\n");
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

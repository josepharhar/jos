#include "kernel/vfs/find_file.h"

#include "string.h"
#include "kernel/printk.h"
#include "kernel/page.h"
#include "asm.h"

namespace vfs {

struct ReadDirCallbackArg {
  Filepath filepath;
  FindFileCallback callback;
  void* callback_arg;
};

static void ReadDirCallback(stdj::Array<Inode*> inodes, void* void_arg) {
  printk("vfs::ReadDirCallback\n");
  ReadDirCallbackArg* arg = (ReadDirCallbackArg*)void_arg;

  if (!arg->filepath.Size()) {
    arg->callback(0, arg->callback_arg);
    delete arg;
    return;
  }
  stdj::string target_filename = arg->filepath.RemoveFirst();

  // TODO delete all inodes or make them unique_ptrs
  for (int i = 0; i < inodes.Size(); i++) {
    Inode* inode = inodes.Get(i);
    if (!strcmp(inode->GetName(), target_filename.c_str())) {
      // found it
      printk("vfs::ReadDirCallback found \"%s\"\n", target_filename.c_str());
      if (!arg->filepath.Size()) {
        printk("vfs::ReadDirCallback success!\n");
        arg->callback(inode, arg->callback_arg);
        delete arg;
        return;

      } else if (inode->IsDirectory()) {
        printk("vfs::ReadDirCallback entering directory\n");
        inode->ReadDir(ReadDirCallback, arg);
        return;
      }
    }
  }

  printk("vfs::ReadDirCallback failed to find \"%s\"\n", target_filename.c_str());
  arg->callback(0, arg->callback_arg);
  delete arg;
  return;
}

void FindFile(Inode* root_inode,
              Filepath filepath,
              FindFileCallback callback,
              void* callback_arg) {
  /*(printk("calling kmalloc(8000)\n");
  printk("kmalloc(8000): %p\n", kmalloc(8000));*/
  //kmalloc(50);
  /*uint64_t asdf = 0x800000010100;
  printk("phys of %p: %p\n", asdf, page::GetPhysicalAddress(Getcr3(), asdf));*/
  for (int i = 0; i < 10; i++) {
  uint64_t* asdf = (uint64_t*)kmalloc(10);
    printk("kmalloc(10): %p\n", asdf);
    printk("*asdf = 0x1234\n");
    *asdf = 0x1234;
    printk("*asdf: %p\n", *asdf);
  }
  printk("done rofling\n");
  printk("vfs::FindFile filepath: %s\n", filepath.ToString().c_str());
  stdj::Array<stdj::string> filepath_array = filepath.GetArray();

  if (filepath_array.Size() == 0) {
    printk("vfs::FindFile filepath_array.Size() == 0\n");
    callback(0, callback_arg);

  } else {
    printk("vfs::FindFile calling ReadDir()\n");
    ReadDirCallbackArg* arg = new ReadDirCallbackArg();
    arg->filepath = filepath;
    arg->callback = callback;
    arg->callback_arg = callback_arg;

    root_inode->ReadDir(ReadDirCallback, arg);
  }
}

}  // namespace vfs

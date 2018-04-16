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
  printk("vfs::ReadDirCallback arg->filepath ptr: %p\n", &(arg->filepath));
  {
    stdj::string filepath_string = arg->filepath.ToString();
    printk("vfs::ReadDirCallback arg->filepath: %s\n", filepath_string.c_str());
  }
  stdj::string target_filename = arg->filepath.RemoveFirst();
  printk("vfs::ReadDirCallback target_filename: %s\n", target_filename.c_str());
  {
    stdj::string filepath_string = arg->filepath.ToString();
    printk("vfs::ReadDirCallback arg->filepath after remove: %s\n",
           filepath_string.c_str());
  }

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

  printk("vfs::ReadDirCallback failed to find \"%s\"\n",
         target_filename.c_str());
  arg->callback(0, arg->callback_arg);
  delete arg;
  return;
}

void FindFile(Inode* root_inode,
              Filepath filepath,
              FindFileCallback callback,
              void* callback_arg) {
  printk("one\n");
  printk("vfs::FindFile filepath: %s\n", filepath.ToString().c_str());
  stdj::Array<stdj::string> filepath_array = filepath.GetArray();
  printk("vfs::FindFile filepath_array.Data(): %p\n", filepath_array.Data());

  if (filepath_array.Size() == 0) {
    printk("vfs::FindFile filepath_array.Size() == 0\n");
    callback(0, callback_arg);

  } else {
    printk("vfs::FindFile calling ReadDir()\n");
    ReadDirCallbackArg* arg = new ReadDirCallbackArg();
    arg->filepath = filepath;
    {
      stdj::string filepath_string = arg->filepath.ToString();
      printk("vfs::FindFile arg->filepath ptr: %p\n", &(arg->filepath));
      printk("vfs::FindFile arg->filepath: %s\n", filepath_string.c_str());
    }
    arg->callback = callback;
    arg->callback_arg = callback_arg;

    root_inode->ReadDir(ReadDirCallback, arg);
  }
  printk("vfs::FindFile filepath_array.Data(): %p\n", filepath_array.Data());
  for (int i = 0; i < filepath_array.Size(); i++) {
    printk("filepath_array.Get(%d).Data(): %p\n", i,
           filepath_array.Get(i).Data());
  }
}

}  // namespace vfs

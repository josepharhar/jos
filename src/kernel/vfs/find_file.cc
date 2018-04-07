#include "kernel/vfs/find_file.h"

#include "string.h"

namespace vfs {

struct ReadDirCallbackArg {
  Filepath filepath;
  FindFileCallback callback;
  void* callback_arg;
};

static void ReadDirCallback(stdj::Array<Inode*> inodes, void* void_arg) {
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
      if (!arg->filepath.Size()) {
        arg->callback(inode, arg->callback_arg);
        delete arg;
        return;

      } else if (inode->IsDirectory()) {
        inode->ReadDir(ReadDirCallback, arg);
        return;
      }
    }
  }

  arg->callback(0, arg->callback_arg);
  delete arg;
  return;
}

void FindFile(Inode* root_inode,
              Filepath filepath,
              FindFileCallback callback,
              void* callback_arg) {
  stdj::Array<stdj::string> filepath_array = filepath.GetArray();

  if (filepath_array.Size() == 0) {
    callback(0, callback_arg);

  } else {
    ReadDirCallbackArg* arg = new ReadDirCallbackArg();
    arg->filepath = filepath;
    arg->callback = callback;
    arg->callback_arg = callback_arg;

    root_inode->ReadDir(ReadDirCallback, arg);
  }
}

}  // namespace vfs

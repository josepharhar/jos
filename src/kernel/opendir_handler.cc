#include "opendir_handler.h"

#include "syscall.h"
#include "syscall_handler.h"
#include "shared/dirent.h"
#include "kernel/vfs/globals.h"

// this isn't great and isn't very secure, but other options are worse.
static uint64_t next_handle_id = 1;
struct OpendirHandle {
  OpendirHandle(uint64_t new_id) : id(new_id) {}
  uint64_t id;
  stdj::Array<vfs::Inode*> inodes;
  int inode_index;
};
static const OpendirHandle NULL_HANDLE;
static stdj::Map<uint64_t, OpendirHandle*, NULL_HANDLE>* id_to_handle;

struct OpendirContext {
  proc::ProcContext* proc;
  proc::BlockedQueue proc_queue;
  vfs::Filepath filepath;
  vfs::Filepath original_filepath;
  SyscallOpendirParams* params;
};

static void Return(
    OpendirContext* arg,
    bool success,
    stdj::Array<vfs::Inode*> inodes = stdj::Array<vfs::Inode*>()) {
  uint64_t old_cr3 = Getcr3();
  bool switch_tables = old_cr3 == arg->proc->cr3;

  if (switch_tables) {
    Setcr3(arg->proc->cr3);
  }

  arg->params->success_writeback = success;
  if (success) {
    uint64_t new_handle_id = next_handle_id++;
    OpendirHandle* new_handle = new OpendirHandle(new_handle_id);
    new_handle->inodes = inodes;
    new_handle->inode_index = 0;

    id_to_handle.Set(new_handle_id, new_handle);

    arg->params->id_writeback = new_handle_id;
  }

  if (switch_tables) {
    Setcr3(old_cr3);
  }

  arg->proc_queue.UnblockHead();
}

static void ReadDirCallback(stdj::Array<vfs::Inode*> inodes, void* void_arg) {
  OpendirContext* arg = (OpendirContext*)void_arg;

  if (!arg->filepath.Size()) {
    // we are at the target directory, return this list of inodes
    Return(arg, true, inodes);
    return;
  }
  stdj::string target_filename = arg->filepath.RemoveFirst();

  for (int i = 0; i < inodes.Size(); i++) {
    Inode* inode = inodes.Get(i);
    if (!strcmp(inode->GetName(), target_filename.c_str())) {
      // found it
      // we need to go deeper
      inode->ReadDir(ReadDirCallback, arg);
      return;
    }
  }

  stdj::string original_filepath_string = arg->original_filepath.ToString();
  printk("opendir ReadDirCallback failed to find \"%s\"\n",
         original_filepath_string.c_str());
  Return(arg, false);
  return;
}

static void HandleSyscallOpendir(uint64_t interrupt_number,
                                 uint64_t param_1,
                                 uint64_t param_2,
                                 uint64_t param_3) {
  SyscallOpendirParams* params = (SyscallOpendirParams*)param_1;

  OpendirContext* arg = new OpendirContext();
  arg->proc = proc::GetCurrentProc();
  arg->proc_queue.BlockCurrentProcNoNesting();
  arg->filepath = vfs::Filepath(params->filepath);
  arg->original_filepath = arg->filepath;
  arg->params = params;
  vfs::GetRootDirectory()->ReadDir(ReadDirCallback, arg);
}

static void HandleSyscallReaddir(uint64_t interrupt_number,
                                 uint64_t param_1,
                                 uint64_t param_2,
                                 uint64_t param_3) {
  SyscallReaddirParams* params = (SyscallReaddirParams)param_1;
  if (!id_to_handle->ContainsKey(params->id)) {
    // bad request
    params->success_writeback = false;
    return;
  }

  OpendirHandle* handle = id_to_handle->Get(params->id);
  if (handle->inode_index >= handle->inodes.Size()) {
    // ran out, this is OK but we have to return... what??
    // TODO TODO TODO
  }
  vfs::Inode next_inode = handle->inodes.Get(handle->inode_index++);
}


static void HandleSyscallClosedir(uint64_t interrupt_number,
                                 uint64_t param_1,
                                 uint64_t param_2,
                                 uint64_t param_3) {
  // TODO who will this get called when a dir is still open
  // and a proc exits???????
}

void InitOpendir() {
  id_to_handle = new stdj::Map<uint64_t, OpendirHandle, NULL_HANDLE>();
  SetSyscallHandler(SYSCALL_OPENDIR, HandleSyscallOpendir);
  SetSyscallHandler(SYSCALL_READDIR, HandleSyscallReaddir);
  SetSyscallHandler(SYSCALL_CLOSEDIR, HandleSyscallClosedir);
}

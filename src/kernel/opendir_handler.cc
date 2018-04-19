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
};
static const OpendirHandle NULL_HANDLE;
static stdj::Map<uint64_t, OpendirHandle, NULL_HANDLE>* id_to_handle;

struct OpendirContext {
  proc::ProcContext* proc;
  proc::BlockedQueue proc_queue;
  vfs::Filepath dir_to_read;
};
static void ReadDirCallback(stdj::Array<vfs::Inode*> inodes, void* void_arg) {
  OpendirContext* context = (OpendirContext*)void_arg;
}

static void HandleSyscallOpendir(uint64_t interrupt_number,
                          uint64_t param_1,
                          uint64_t param_2,
                          uint64_t param_3) {
  SyscallOpendirParams* params = (SyscallOpendirParams*)param_1;

  OpendirContext* arg = new OpendirContext();
  arg->proc = proc::GetCurrentProc();
  arg->proc_queue.BlockCurrentProcNoNesting();
  vfs::GetRootDirectory()->ReadDir(ReadDirCallback, arg);

  //OpendirHandle dir_

  /*uint64_t new_handle_id = next_handle_id++;
  OpendirHandle new_handle(new_handle_id);
  params->dir.id = next_handle_id++;
  params->success_writeback = true;*/
}

void InitOpendir() {
  id_to_handle = new stdj::Map<uint64_t, OpendirHandle, NULL_HANDLE>();
  SetSyscallHandler(SYSCALL_OPENDIR, HandleSyscallOpendir);
  SetSyscallHandler(SYSCALL_READDIR, HandleSyscallReaddir);
  SetSyscallHandler(SYSCALL_CLOSEDIR, HandleSyscallClosedir);
}

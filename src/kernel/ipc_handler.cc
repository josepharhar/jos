#include "ipc_handler.h"

#include "syscall_handler.h"
#include "syscall.h"
#include "proc.h"
#include "shared/unistd.h"
#include "buffer_file.h"
#include "page.h"
#include "printk.h"
#include "asm.h"

namespace ipc {

static void HandleSyscallWrite(uint64_t interrupt_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3) {
  // TODO security this
  SyscallRdWrParams* params = (SyscallRdWrParams*)param_1;

  Pipe* pipe = proc::GetPipeForFdFromCurrentProc(params->fd);
  if (!pipe) {
    // fd must have been invalid
    printk("HandleSyscallWrite() invalid fd: %d, listing fds:\n", params->fd);
    proc::FdMap* fd_map = &(proc::GetCurrentProc()->fd_map_);
    // for (int i = 0; i < fd_map->GetKeyAt(i))
    for (int i = 0; i < fd_map->Size(); i++) {
      int key = fd_map->GetKeyAt(i);
      ipc::Pipe* value = fd_map->Get(key);
      printk("  %d: %p\n", key, value);
    }
  } else {
    pipe->Write(params->buffer, params->size, &(params->size_writeback));
  }
}

static void HandleSyscallRead(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  // TODO security this
  SyscallRdWrParams* params = (SyscallRdWrParams*)param_1;

  Pipe* pipe = proc::GetPipeForFdFromCurrentProc(params->fd);
  pipe->Read(params->buffer, params->size, &(params->size_writeback));
}

static void HandleSyscallPipe(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  // TODO security this
  int* pipefd = (int*)param_1;
  // pipefd[0] is read end
  // pipefd[1] is write end

  // TODO make global list of open files?
  BufferFile* new_file = new BufferFile();
  Pipe* read_pipe = new_file->Open(RDONLY);
  Pipe* write_pipe = new_file->Open(WRONLY);

  int read_fd = proc::AddPipeToProc(proc::GetCurrentProc(), read_pipe);
  int write_fd = proc::AddPipeToProc(proc::GetCurrentProc(), write_pipe);

  pipefd[0] = read_fd;
  pipefd[1] = write_fd;
}

static void HandleSyscallClose(uint64_t interrupt_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3) {
  int fd = (int)param_1;

  printk("kernel pid: %d cr3: %p\n", proc::GetCurrentProc()->pid, Getcr3());
}

void Init() {
  SetSyscallHandler(SYSCALL_WRITE, HandleSyscallWrite);
  SetSyscallHandler(SYSCALL_READ, HandleSyscallRead);
  SetSyscallHandler(SYSCALL_PIPE, HandleSyscallPipe);
  SetSyscallHandler(SYSCALL_CLOSE, HandleSyscallClose);
}

}  // namespace ipc

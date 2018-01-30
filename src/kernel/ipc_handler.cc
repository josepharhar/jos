#include "ipc_handler.h"

#include "syscall_handler.h"
#include "syscall.h"
#include "proc.h"
#include "shared/ipc.h"
#include "buffer_file.h"
#include "page.h"
#include "printk.h"

namespace ipc {

static void HandleSyscallWrite(uint64_t interrupt_number,
                               uint64_t param_1,
                               uint64_t param_2,
                               uint64_t param_3) {
  // TODO security this
  SyscallRdWrParams* params = (SyscallRdWrParams*)param_1;

  Pipe* pipe = proc::GetPipeForFdFromCurrentProc(params->fd);
  params->size_writeback = pipe->Write(params->buffer, params->size);
}

static void HandleSyscallRead(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  // TODO security this
  SyscallRdWrParams* params = (SyscallRdWrParams*)param_1;

  Pipe* pipe = proc::GetPipeForFdFromCurrentProc(params->fd);
  params->size_writeback = pipe->Read(params->buffer, params->size);
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

  int read_fd = proc::AddPipeToCurrentProc(read_pipe);
  int write_fd = proc::AddPipeToCurrentProc(write_pipe);

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

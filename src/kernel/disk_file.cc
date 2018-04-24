#include "disk_file.h"

#include "proc.h"
#include "asm.h"
#include "printk.h"

// TODO delete file_ when done or something
DiskFile::DiskFile(vfs::File* file) : file_(file) {}

ipc::Pipe* DiskFile::Open(ipc::Mode mode) {
  DiskPipe* pipe = new DiskPipe(this, mode);
  pipes_.Add(pipe);
  return pipe;
}

void DiskFile::Close(ipc::Pipe* pipe) {
  pipes_.RemoveValue(pipe);
  delete pipe;
}

int DiskFile::GetNumPipes() {
  return pipes_.Size();
}

struct ReadFileContext {
  proc::ProcContext* proc;
  proc::BlockedQueue blocked_queue;
  void* read_kbuffer;
  int* size_writeback;
  int size_to_read;
  void* dest_buffer;
};

static void ReadFileCallback(void* void_arg) {
  ReadFileContext* arg = (ReadFileContext*)void_arg;

  uint64_t old_cr3 = Getcr3();
  bool switch_tables = old_cr3 != arg->proc->cr3;
  if (switch_tables) {
    Setcr3(arg->proc->cr3);
  }

  memcpy(arg->dest_buffer, arg->read_kbuffer, arg->size_to_read);
  *(arg->size_writeback) = arg->size_to_read;

  if (switch_tables) {
    Setcr3(old_cr3);
  }

  arg->blocked_queue.UnblockHead();
  kfree(arg->read_kbuffer);
  delete arg;
}

void DiskFile::Read(ipc::Pipe* pipe,
                    uint8_t* dest_buffer,
                    int read_size,
                    int* size_writeback) {
  // this is being called from reading proc, i guess?

  int size_to_read = file_->GetSize() - (file_->GetOffset() + 1);
  if (!size_to_read) {
    // end of file
    *size_writeback = 0;
    return;
  }

  if (read_size < size_to_read) {
    size_to_read = read_size;
  }
  void* read_kbuffer = kmalloc(size_to_read);

  ReadFileContext* arg = new ReadFileContext();
  arg->proc = proc::GetCurrentProc();
  arg->blocked_queue.BlockCurrentProcNoNesting();
  arg->read_kbuffer = read_kbuffer;
  arg->size_writeback = size_writeback;
  arg->size_to_read = size_to_read;
  arg->dest_buffer = dest_buffer;
  if (file_->Read(read_kbuffer, size_to_read, ReadFileCallback, arg)) {
    printk("DiskFile::Read file_->Read() failed!\n");
  }
}

void DiskFile::Write(ipc::Pipe* pipe,
                     const uint8_t* src_buffer,
                     int write_size,
                     int* size_writeback) {}

DiskPipe::DiskPipe(ipc::File* file, ipc::Mode mode) : ipc::Pipe(file, mode) {}

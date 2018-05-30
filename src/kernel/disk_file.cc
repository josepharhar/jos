#include "disk_file.h"

#include "proc.h"
#include "asm.h"
#include "printk.h"

// TODO delete file_ when done or something
DiskFile::DiskFile(vfs::File* file) : file_(file) {}

ipc::Pipe* DiskFile::Open(ipc::Mode mode) {
  ipc::Pipe* pipe = new ipc::Pipe(this, mode);
  pipes_.Add(pipe);
  return pipe;
}

void DiskFile::Close(ipc::Pipe* pipe) {
  pipes_.RemoveValue(pipe);
}

int DiskFile::GetNumPipes() {
  return pipes_.Size();
}

struct ReadFileContext {
  proc::ProcContext* proc;
  proc::BlockedQueue blocked_queue;
  void* read_kbuffer;
  int* size_writeback;
  int read_size;
  void* dest_buffer;
};

static void ReadFileCallback(bool success, void* void_arg) {
  ReadFileContext* arg = (ReadFileContext*)void_arg;

  uint64_t old_cr3 = Getcr3();
  bool switch_tables = old_cr3 != arg->proc->cr3;
  if (switch_tables) {
    Setcr3(arg->proc->cr3);
  }

  if (success) {
    memcpy(arg->dest_buffer, arg->read_kbuffer, arg->read_size);
    *(arg->size_writeback) = arg->read_size;

  } else {
    printk("DiskFile::ReadFileCallback read failed!\n");
    *(arg->size_writeback) = -1;
  }

  if (switch_tables) {
    Setcr3(old_cr3);
  }

  arg->blocked_queue.UnblockHead();
  kfree(arg->read_kbuffer);
  delete arg;
}

// TODO consolidate DiskFile and vfs::File

void DiskFile::Read(ipc::Pipe* pipe,
                    uint8_t* dest_buffer,
                    int read_size,
                    int* size_writeback) {
  // this is being called from reading proc, i guess?

  int size_remaining_in_file = file_->GetSize() - file_->GetOffset();
  /*printk("size_remaining_in_file: %d\n", size_remaining_in_file);
  printk("  file_->GetSize(): %d, file_->GetOffset(): %d\n",
      file_->GetSize(), file_->GetOffset());*/
  if (size_remaining_in_file < 1) {
    // end of file
    *size_writeback = 0;
    return;
  }

  if (read_size > size_remaining_in_file) {
    read_size = size_remaining_in_file;
  }
  void* read_kbuffer = kmalloc(read_size);

  ReadFileContext* arg = new ReadFileContext();
  arg->proc = proc::GetCurrentProc();
  arg->blocked_queue.BlockCurrentProcNoNesting();
  arg->read_kbuffer = read_kbuffer;
  arg->size_writeback = size_writeback;
  arg->read_size = read_size;
  arg->dest_buffer = dest_buffer;
  if (file_->Read(read_kbuffer, read_size, ReadFileCallback, arg)) {
    printk("DiskFile::Read file_->Read() failed!\n");
  }
}

void DiskFile::Write(ipc::Pipe* pipe,
                     const uint8_t* src_buffer,
                     int write_size,
                     int* size_writeback) {
  // TODO implement fat32 writing
}

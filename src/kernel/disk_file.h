#ifndef DISK_FILE_H_
#define DISK_FILE_H_

#include "ipc.h"
#include "jarray.h"
#include "kernel/vfs/file.h"

class DiskFile : public ipc::File {
 public:
  DiskFile(vfs::File* file);

  ipc::Pipe* Open(ipc::Mode mode) override;
  void Close(ipc::Pipe* pipe) override;
  int GetNumPipes() override;

  void Write(ipc::Pipe* pipe,
             const uint8_t* source_buffer,
             int write_size,
             int* size_writeback) override;
  void Read(ipc::Pipe* pipe,
            uint8_t* dest_buffer,
            int read_size,
            int* size_writeback) override;

 private:
  stdj::Array<ipc::Pipe*> pipes_;
  vfs::File* file_;
};

class DiskPipe : public ipc::Pipe {
 public:
  DiskPipe(ipc::File* file, ipc::Mode mode);
};

#endif  // DISK_FILE_H_

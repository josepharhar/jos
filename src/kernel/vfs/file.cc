#include "kernel/vfs/file.h"

#include "kernel/vfs/inode.h"
#include "kernel/vfs/superblock.h"

namespace vfs {

File::File(Inode* inode) : inode(inode), offset(0) {}

struct ReadReadClusterArg {
  File* file;
  uint8_t* buffer;
  uint8_t* dest;
  uint64_t length;
};
// static
void File::ReadReadCluster(void* void_arg) {
  ReadReadClusterArg* arg = (ReadReadClusterArg*)void_arg;

  for (int i = 0; i < 512 && arg->length; i++) {
    *arg->dest++ = arg->buffer[i];
    arg->length--;
  }
  cluster = inode->GetSuperblock()->GetNextCluster(cluster);

  if (length) {
    arg->inode->GetSuperblock()->ReadCluster(arg->cluster, ReadReadCluster, arg);
  } else {
    arg->file->offset += length;
    kfree(arg->buffer);
    kfree(arg);
    arg->callback();
  }
}

int File::Read(uint8_t* dest, uint64_t length, void (*callback)()) {
  if (offset + length > inode->GetSize()) {
    callback();
    return 1;
  }

  uint64_t cluster = inode->GetCluster();
  uint64_t offset_remaining = offset;
  while (offset_remaining > 512) {
    cluster = inode->GetSuperblock()->GetNextCluster(cluster);
    offset_remaining -= 512;
  }

  if (length) {
    ReadReadClusterArg* arg = new ReadReadClusterArg();
    arg->file = this;
    arg->buffer = (uint8_t*)kmalloc(512);
    arg->dest = dest;
    inode->GetSuperblock()->ReadCluster(cluster, ReadReadCluster, arg);
  } else {
    callback();
  }
  return 0;
}

int File::Write(uint8_t* src, uint64_t length, void (*callback)()) {
  // TODO
  return 1;
}

int File::Seek(uint64_t offset) {
  if (offset >= inode->GetSize()) {
    return 1;
  }
  this->offset = offset;
  return 0;
}

int File::Close() {
  // TODO what should be done here
  return 0;
}

uint64_t File::GetSize() {
  return inode->GetSize();
}

}  // namespace vfs

#include "kernel/vfs/file.h"

#include "kernel/vfs/inode.h"
#include "kernel/vfs/superblock.h"

namespace vfs {

File::File(Inode* inode) : inode(inode), offset(0) {}

int File::Read(uint8_t* dest, uint64_t length) {
  if (offset + length > inode->GetSize()) {
    return 1;
  }

  uint64_t cluster = inode->GetCluster();
  uint64_t offset_remaining = offset;
  while (offset_remaining > 512) {
    cluster = inode->GetSuperblock()->GetNextCluster(cluster);
    offset_remaining -= 512;
  }

  while (length) {
    uint8_t* buffer = inode->GetSuperblock()->ReadCluster(cluster);
    for (int i = 0; i < 512 && length; i++) {
      *dest++ = buffer[i];
      length--;
    }
    kfree(buffer);
    cluster = inode->GetSuperblock()->GetNextCluster(cluster);
  }

  offset += length;

  return 0;
}

int File::Write(uint8_t* src, uint64_t length) {
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

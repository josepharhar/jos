#ifndef VFS_SUPERBLOCK_H_
#define VFS_SUPERBLOCK_H_

#include "kernel/vfs/vfs2.h"

namespace vfs {

class Superblock {
 public:
  static Superblock* Create(ATADevice* ata_device);
  static void Destroy(Superblock* superblock);

  Inode* GetRootInode();

  // TODO
  /*int SyncFS(); // sync files to disk?
  void PutSuper(); // write superblock to disk*/

  // FAT32 specific information
  uint8_t* ReadCluster(uint64_t cluster);  // TODO this should be given a buffer
  uint64_t GetNextCluster(uint64_t cluster);

 private:
  Inode* root_inode;
  // TODO
  /*char* name; // untitled
  char* type; // fat32*/

  // FAT32 specific information

  uint64_t GetFatsStartSector();
  uint64_t GetDataRegionStartSector();
  uint64_t ClusterToSector(uint64_t cluster_number);

  ATADevice* ata_device;
  MBR mbr;
  MBR partition;
  uint32_t* fat_table;
};

}  // namespace vfs

#endif  // VFS_SUPERBLOCK_H_

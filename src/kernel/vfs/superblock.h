#ifndef VFS_SUPERBLOCK_H_
#define VFS_SUPERBLOCK_H_

#include "kernel/vfs/vfs2.h"

namespace vfs {

typedef void (*SuperblockReadyCallback)(Superblock*);

class Superblock {
 public:
  static void Create(ATADevice* ata_device, SuperblockReadyCallback callback);
  static void Destroy(Superblock* superblock);

  Inode* GetRootInode();

  // TODO
  /*int SyncFS(); // sync files to disk?
  void PutSuper(); // write superblock to disk*/

  // FAT32 specific information
  void ReadCluster(uint64_t cluster,
                   void* dest,
                   ATARequestCallback callback,
                   void* arg = 0);
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

  // temp storage for events in Superblock::Create
  SuperblockReadyCallback ready_callback_;
  int boot_record_block_index_;
  uint8_t* fat_table_bytes_;

  // static helper callbacks that need private visibility
  static void SuperblockCreateReadBootRecord(void* arg);
  static void SuperblockCreateReadLbaFirstSector(void* arg);
  static void SuperblockCreateMBRScanned(void* arg);
};

}  // namespace vfs

#endif  // VFS_SUPERBLOCK_H_

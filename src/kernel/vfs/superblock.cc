#include "kernel/vfs/superblock.h"

#include "kernel/vfs/inode.h"
#include "kernel/printk.h"

namespace vfs {

// static
void Superblock::SuperblockCreateReadBootRecord(void* arg) {
  Superblock* superblock = (Superblock*)arg;

  if (superblock->boot_record_block_index_ <
      superblock->partition.boot_record.sectors_per_fat) {
    superblock->ata_device->ReadBlock(
        superblock->GetFatsStartSector() + superblock->boot_record_block_index_,
        superblock->fat_table_bytes_ +
            512 * superblock->boot_record_block_index_,
        SuperblockCreateReadBootRecord, superblock);
    superblock->boot_record_block_index_++;
    return;
  }

  superblock->fat_table = (uint32_t*)superblock->fat_table_bytes_;
  superblock->root_inode = (Inode*)kcalloc(sizeof(Inode));
  *superblock->root_inode =
      Inode(superblock->partition.boot_record.root_cluster, (char*)"", true,
            superblock);

  superblock->ready_callback_(superblock);
}

// static
void Superblock::SuperblockCreateReadLbaFirstSector(void* arg) {
  Superblock* superblock = (Superblock*)arg;

  if (superblock->partition.boot_partition_signature !=
      BOOT_SIGNATURE_LITTLE_ENDIAN) {
    printk("invalid boot partition signature: 0x%X\n",
           superblock->partition.boot_partition_signature);
    superblock->ready_callback_(0);
    kfree(superblock);
    return;
  }

  superblock->boot_record_block_index_ = 0;
  superblock->fat_table_bytes_ = (uint8_t*)kmalloc(
      512 * superblock->partition.boot_record.sectors_per_fat);
  SuperblockCreateReadBootRecord(superblock);
}

// static
void Superblock::SuperblockCreateMBRScanned(void* arg) {
  Superblock* superblock = (Superblock*)arg;

  uint8_t first_partition_status = superblock->mbr.partition_entries[0].status;
  if (first_partition_status != MBR_STATUS_ACTIVE) {
    printk("invalid first partition status: 0x%X\n", first_partition_status);
    superblock->ready_callback_(0);
    kfree(superblock);
    return;
  }
  uint8_t first_partition_type =
      superblock->mbr.partition_entries[0].partition_type;
  if (first_partition_type != 0xC) {
    printk("invalid first partition type, should be 0xC for Fat32/LBA: 0x%X\n",
           first_partition_type);
    superblock->ready_callback_(0);
    kfree(superblock);
    return;
  }

  uint64_t lba_first_sector =
      superblock->mbr.partition_entries[0].lba_first_sector;

  superblock->ata_device->ReadBlock(lba_first_sector, &superblock->partition,
                                    SuperblockCreateReadLbaFirstSector,
                                    superblock);
}

// static
void Superblock::Create(ATADevice* ata_device,
                        SuperblockReadyCallback callback) {
  Superblock* superblock = (Superblock*)kcalloc(sizeof(Superblock));
  superblock->ata_device = ata_device;
  superblock->ready_callback_ = callback;

  // scan MBR
  ata_device->ReadBlock(0, &superblock->mbr, SuperblockCreateMBRScanned,
                        superblock);
}

// static
void Superblock::Destroy(Superblock* superblock) {
  kfree(superblock->root_inode);
  kfree(superblock->fat_table);
  kfree(superblock);
}

Inode* Superblock::GetRootInode() {
  return root_inode;
}

uint64_t Superblock::GetFatsStartSector() {
  return mbr.partition_entries[0].lba_first_sector +
         partition.boot_record.bpb.num_reserved_sectors;
}

uint64_t Superblock::GetDataRegionStartSector() {
  return GetFatsStartSector() +
         partition.boot_record.sectors_per_fat *
             partition.boot_record.bpb.num_fats;
}

uint64_t Superblock::ClusterToSector(uint64_t cluster_number) {
  // reserved sectors, then FATs, then data region
  // clusters and sectors are 512 bytes, so ignore the difference
  return GetDataRegionStartSector() + cluster_number - 2;
}

void Superblock::ReadCluster(uint64_t cluster,
                             void* dest,
                             ATARequestCallback callback,
                             void* arg) {
  ata_device->ReadBlock(ClusterToSector(cluster), dest, callback, arg);
}

// returns zero if there is no next cluster
uint64_t Superblock::GetNextCluster(uint64_t cluster) {
  uint64_t next_cluster = fat_table[cluster];
  if (IsValidCluster(next_cluster)) {
    return next_cluster;
  } else {
    return 0;
  }
}

}  // namespace vfs

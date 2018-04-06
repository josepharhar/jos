#include "kernel/vfs/superblock.h"

namespace vfs {

// static
Superblock* Superblock::Create(ATADevice* ata_device) {
  Superblock* superblock = (Superblock*)kcalloc(sizeof(Superblock));
  superblock->ata_device = ata_device;

  // scan MBR
  ata_device->ReadBlock(0, &superblock->mbr);

  uint8_t first_partition_status = superblock->mbr.partition_entries[0].status;
  if (first_partition_status != MBR_STATUS_ACTIVE) {
    printk("invalid first partition status: 0x%X\n", first_partition_status);
    return 0;
  }
  uint8_t first_partition_type =
      superblock->mbr.partition_entries[0].partition_type;
  if (first_partition_type != 0xC) {
    printk("invalid first partition type, should be 0xC for Fat32/LBA: 0x%X\n",
           first_partition_type);
    return 0;
  }

  uint64_t lba_first_sector =
      superblock->mbr.partition_entries[0].lba_first_sector;

  ata_device->ReadBlock(lba_first_sector, &superblock->partition);

  if (superblock->partition.boot_partition_signature !=
      BOOT_SIGNATURE_LITTLE_ENDIAN) {
    printk("invalid boot partition signature: 0x%X\n",
           superblock->partition.boot_partition_signature);
    return 0;
  }

  uint8_t* fat_table_bytes = (uint8_t*)kmalloc(
      512 * superblock->partition.boot_record.sectors_per_fat);
  for (int i = 0; i < superblock->partition.boot_record.sectors_per_fat; i++) {
    ata_device->ReadBlock(superblock->GetFatsStartSector() + i,
                          fat_table_bytes + 512 * i);
  }
  superblock->fat_table = (uint32_t*)fat_table_bytes;

  superblock->root_inode = (Inode*)kcalloc(sizeof(Inode));
  *superblock->root_inode =
      Inode(superblock->partition.boot_record.root_cluster, (char*)"", true,
            superblock);

  return superblock;
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

uint8_t* Superblock::ReadCluster(uint64_t cluster) {
  uint8_t* sector = (uint8_t*)kmalloc(512);
  ata_device->ReadBlock(ClusterToSector(cluster), sector);
  return sector;
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

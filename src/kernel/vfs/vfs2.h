#ifndef VFS_VFS2_H_
#define VFS_VFS2_H_

#include "stdint.h"
#include "ata.h"
#include "kernel/kmalloc.h"

#define LFN_BUFFER_LENGTH 257

#define PARTITION_ENTRY_OFFSET_1 446
#define PARTITION_ENTRY_OFFSET_2 462
#define PARTITION_ENTRY_OFFSET_3 478
#define PARTITION_ENTRY_OFFSET_4 494

#define MBR_STATUS_ACTIVE 0x80
#define MBR_STATUS_INACTIVE 0x0

#define BOOT_SIGNATURE_LITTLE_ENDIAN 0xAA55

#define FILE_ATTRIBUTE_READ_ONLY 0x01
#define FILE_ATTRIBUTE_HIDDEN 0x02
#define FILE_ATTRIBUTE_SYSTEM 0x04
#define FILE_ATTRIBUTE_VOLUME_ID 0x08
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_ARCHIVE 0x20
#define FILE_ATTRIBUTE_DEVICE 0x40
#define FILE_ATTRIBUTE_RESERVED 0x80
// LFN = Long File Name
//#define FILE_ATTRIBUTE_LFN (FILE_ATTRIBUTE_READ_ONLY | FILE_ATTRIBUTE_HIDDEN |
// FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VOLUME_ID)
#define FILE_ATTRIBUTE_LFN 0xF

// the highest 4 bits are ignored when reading clusters from fat table
#define FAT32_CLUSTER_MASK 0x0FFFFFFF

// if a cluster number is in these values inclusive,
// then it referes to another cluster in the file's linked list
// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Cluster_values
#define FAT32_CLUSTER_VALID_START 0x2
#define FAT32_CLUSTER_VALID_END 0x0FFFFFEF

#define DIRECTORY_ENTRIES_PER_SECTOR (512 / sizeof(DirectoryEntry))

// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system

namespace vfs {

// must be 32 bytes
struct DirectoryEntry {
  char legacy_name[11];
  uint8_t attributes;
  uint8_t windows_nt;
  uint8_t creation_time_ms;
  uint8_t creation_time_hms[2];
  uint8_t creation_date[2];
  uint8_t accessed_date[2];
  uint16_t first_cluster_number_high;  // high 16 bits of this entry's first
                                       // cluster number
  uint8_t last_modified_time[2];
  uint8_t last_modified_date[2];
  uint16_t first_cluster_number_low;
  uint32_t filesize;
} __attribute__((packed));

// must be 32 bytes
struct DirectoryEntryLFN {
  uint8_t index;
  uint16_t name_1[5];
  uint8_t attribute;        // must be 0xF
  uint8_t long_entry_type;  // zero for name entries
  uint8_t checksum;
  uint16_t name_2[6];
  uint16_t zero;
  uint16_t name_3[2];
} __attribute__((packed));

// 16 bytes
// https://en.wikipedia.org/wiki/Master_boot_record#PTE
struct PartitionEntry {
  uint8_t status;      // 0x80 = active/bootable, 0x00 = inactive, else invalid
  uint8_t head_first;  // CHS, ignore
  uint8_t sec_high_first;  // CHS, ignore
  uint8_t sec_low_first;   // CHS, ignore
  uint8_t partition_type;
  uint8_t head_last;      // CHS, ignore
  uint8_t sec_high_last;  // CHS, ignore
  uint8_t sec_low_last;   // CHS, ignore
  uint32_t lba_first_sector;
  uint32_t num_sectors;
} __attribute__((packed));

// http://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
// multi-byte fields are little endian, so uint16_t is ok to use
// 36 bytes
struct BPB {
  uint8_t jmp[3];
  uint8_t oem_id[8];
  uint16_t bytes_per_sector;    // assert this is 512 bytes TODO
  uint8_t sectors_per_cluster;  // FAT table points to clusters, not sectors!
  uint16_t
      num_reserved_sectors;  // reserved section after BPB, in sectors, for GRUB
  uint8_t num_fats;          // number of tables, usually 2
  uint16_t num_directory_entries;
  uint16_t total_sectors;  // if 0 then more than 65535 sectors, count
                           // total_sectors_large instead
  uint8_t media_descriptor_type;
  uint16_t sectors_per_fat;  // fat12/fat16 only. if zero (fat32), then use
                             // BootRecord.sectors_per_fat
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t num_hidden_sectors;   // LBA of the beginning of the partition
  uint32_t total_sectors_large;  // used if total_sectors == 0
} __attribute__((packed));

// FAT32 Extended Boot Record (EBPB)
struct BootRecord {
  struct BPB bpb;
  uint32_t sectors_per_fat;
  uint8_t flags[2];
  uint8_t major_version;
  uint8_t minor_version;
  uint32_t root_cluster;  // cluster number of root directory, usually 2
  uint16_t sector_fsinfo;
  uint16_t sector_backup_boot;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t nt_flags;
  uint8_t signature;  // must be 0x28 or 0x29
  uint32_t volumeid_serial_number;
  char volume_label_string[11];
  char system_id_string[8];  // always "FAT32   "
} __attribute__((packed));

// entire MBR block, 512 bytes
struct MBR {
  struct BootRecord boot_record;
  uint8_t boot_code[420 - (16 * 4)];
  struct PartitionEntry partition_entries[4];
  uint16_t boot_partition_signature;  // 0x55AA in little endian
} __attribute__((packed));

class Inode;
class Superblock;
class File;

bool IsValidCluster(uint64_t cluster);

}  // namespace vfs

#endif  // VFS_VFS2_H_

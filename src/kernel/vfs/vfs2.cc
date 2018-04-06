#include "vfs2.h"

namespace vfs {

// TODO move fat32 implementation to a subclass when subclasses work

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
//FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VOLUME_ID)
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

static bool IsValidCluster(uint64_t cluster) {
  return cluster >= FAT32_CLUSTER_VALID_START &&
         cluster <= FAT32_CLUSTER_VALID_END;
}

}  // namespace vfs

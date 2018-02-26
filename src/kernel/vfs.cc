#include "vfs.h"

#include "printk.h"
#include "string.h"
#include "kmalloc.h"

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
//#define FILE_ATTRIBUTE_LFN (FILE_ATTRIBUTE_READ_ONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VOLUME_ID)
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
  uint16_t first_cluster_number_high; // high 16 bits of this entry's first cluster number
  uint8_t last_modified_time[2];
  uint8_t last_modified_date[2];
  uint16_t first_cluster_number_low;
  uint32_t filesize;
} __attribute__((packed));

// must be 32 bytes
struct DirectoryEntryLFN {
  uint8_t index;
  uint16_t name_1[5];
  uint8_t attribute; // must be 0xF
  uint8_t long_entry_type; // zero for name entries
  uint8_t checksum;
  uint16_t name_2[6];
  uint16_t zero;
  uint16_t name_3[2];
} __attribute__((packed));

static bool IsValidCluster(uint64_t cluster) {
  return cluster >= FAT32_CLUSTER_VALID_START
    && cluster <= FAT32_CLUSTER_VALID_END;
}

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

Inode::Inode(uint64_t cluster, char* name, bool is_directory, Superblock* superblock)
   : cluster(cluster), is_directory(is_directory), superblock(superblock) {
  strncpy(this->filename, name, LFN_BUFFER_LENGTH);
}

// static
void Inode::Destroy(Inode* inode) {
  kfree(inode);
}

bool Inode::IsDirectory() {
  return is_directory;
}

char* Inode::GetName() {
  return filename;
}

uint64_t Inode::GetSize() {
  return size;
}

Superblock* Inode::GetSuperblock() {
  return superblock;
}

uint64_t Inode::GetCluster() {
  return cluster;
}

File* Inode::Open() {
  if (is_directory) {
    return 0;
  }

  File* file = (File*) kmalloc(sizeof(File));
  *file = File(this);
  return file;
}

LinkedList<Inode*>* Inode::ReadDir() {
  LinkedList<Inode*>* list = new LinkedList<Inode*>();

  if (!is_directory) {
    return list;
  }

  static char lfn_filename[LFN_BUFFER_LENGTH];
  int reading_lfn = 0;
  uint64_t current_cluster = cluster;

  while (IsValidCluster(current_cluster)) {
    DirectoryEntry* directory_sector = (DirectoryEntry*) superblock->ReadCluster(current_cluster);

    for (int i = 0; i < DIRECTORY_ENTRIES_PER_SECTOR; i++) {
      DirectoryEntry* entry = directory_sector + i;

      switch (entry->attributes) {

        case FILE_ATTRIBUTE_LFN: {
          if (!reading_lfn) {
            // start reading long filename
            reading_lfn = 1;
            memset(lfn_filename, 0, LFN_BUFFER_LENGTH);
          }

          DirectoryEntryLFN* lfn = (DirectoryEntryLFN*) entry;

          // only first five bits of index count
          int lfn_index = (lfn->index & 0x1F) - 1;
          lfn_index *= 13;

          for (int i = 0; i < 5; i++) {
            lfn_filename[lfn_index + i] = lfn->name_1[i];
          }
          for (int i = 0; i < 6; i++) {
            lfn_filename[lfn_index + i + 5] = lfn->name_2[i];
          }
          for (int i = 0; i < 2; i++) {
            lfn_filename[lfn_index + i + 5 + 6] = lfn->name_3[i];
          }

          break;
        }
      
        case FILE_ATTRIBUTE_ARCHIVE:
        case FILE_ATTRIBUTE_DIRECTORY: {
          Inode* new_inode = (Inode*) kcalloc(sizeof(Inode));
          new_inode->is_directory = entry->attributes == FILE_ATTRIBUTE_DIRECTORY;
          new_inode->superblock = superblock;
          new_inode->size = entry->filesize;

          // set filename
          if (reading_lfn) {
            memcpy(new_inode->filename, lfn_filename, LFN_BUFFER_LENGTH);
            reading_lfn = 0;
          } else {
            int filename_chars_written = 0;
            for (int i = 0; i < 8; i++) {
              if (entry->legacy_name[i] != ' ') {
                new_inode->filename[filename_chars_written++] = entry->legacy_name[i];
              }
            }
            // add file extension
            if (entry->legacy_name[8] != ' ') {
              new_inode->filename[filename_chars_written++] = '.';
            }
            for (int i = 0; i < 3; i++) {
              if (entry->legacy_name[8 + i] != ' ') {
                new_inode->filename[filename_chars_written++] = entry->legacy_name[8 + i];
              }
            }
          }

          /*if (!strcmp(new_inode->filename, "..") || !strcmp(new_inode->filename, ".")) {
          }*/

          // set cluster pointer
          new_inode->cluster = (entry->first_cluster_number_high << 16)
            | entry->first_cluster_number_low;

          list->Add(new_inode);

          break;
        }

        default:
          break;
      }

    }

    kfree(directory_sector);
    current_cluster = superblock->GetNextCluster(current_cluster);
  }

  return list;
}

// static
Superblock* Superblock::Create(ATABlockDevice* ata_device) {
  Superblock* superblock = (Superblock*) kcalloc(sizeof(Superblock));
  superblock->ata_device = ata_device;

  // scan MBR
  ata_device->ReadBlock(0, &superblock->mbr);

  /*uint64_t status_pointer = (uint64_t) &mbr.partition_entries[0].status;
  uint64_t zero_pointer = (uint64_t) &mbr;
  printk("status offset 446: %lld\n", status_pointer - zero_pointer);
  printk("sizeof mbr 512: %d\n", sizeof(struct MBR));*/

  uint8_t first_partition_status = superblock->mbr.partition_entries[0].status;
  if (first_partition_status != MBR_STATUS_ACTIVE) {
    printk("invalid first partition status: 0x%X\n", first_partition_status);
    return 0;
  }
  uint8_t first_partition_type = superblock->mbr.partition_entries[0].partition_type;
  if (first_partition_type != 0xC) {
    printk("invalid first partition type, should be 0xC for Fat32/LBA: 0x%X\n", first_partition_type);
    return 0;
  }

  uint64_t lba_first_sector = superblock->mbr.partition_entries[0].lba_first_sector;
  //printk("lba_first_sector: 0x%X\n", lba_first_sector);
  //printk("num_sectors: 0x%X\n", mbr.partition_entries[0].num_sectors);

  ata_device->ReadBlock(lba_first_sector, &superblock->partition);

  /*printk("bytes_per_sector: 0x%X\n", partition.boot_record.bpb.bytes_per_sector);
  printk("sectors_per_cluster: 0x%X\n", partition.boot_record.bpb.sectors_per_cluster);
  uint64_t total_sectors = partition.boot_record.bpb.total_sectors;
  if (!total_sectors) {
    total_sectors = partition.boot_record.bpb.total_sectors_large;
  }
  printk("total_sectors: %p\n", total_sectors);
  printk("num_fats should be 2: %d\n", partition.boot_record.bpb.num_fats);
  printk("system_id_string (should be \"FAT32   \"): \"");
  for (int i = 0; i < 8; i++) {
    printk("%c", partition.boot_record.system_id_string[i]);
  }
  printk("\"\n");
  printk("volume label string: \"");
  for (int i = 0; i < 11; i++) {
    printk("%c", partition.boot_record.volume_label_string[i]);
  }
  printk("\"\n");*/

  if (superblock->partition.boot_partition_signature != BOOT_SIGNATURE_LITTLE_ENDIAN) {
    printk("invalid boot partition signature: 0x%X\n", superblock->partition.boot_partition_signature);
    return 0;
  }

  /*printk("num reserved sectors: 0x%X\n", partition.boot_record.bpb.num_reserved_sectors);
  uint64_t fats_sector = lba_first_sector + partition.boot_record.bpb.num_reserved_sectors;
  printk("sectors per fat: 0x%X\n", partition.boot_record.sectors_per_fat);
  uint64_t root_cluster = partition.boot_record.root_cluster;
  printk("root cluster: 0x%X\n", root_cluster);*/

  uint8_t* fat_table_bytes = (uint8_t*) kmalloc(512 * superblock->partition.boot_record.sectors_per_fat);
  for (int i = 0; i < superblock->partition.boot_record.sectors_per_fat; i++) {
    ata_device->ReadBlock(superblock->GetFatsStartSector() + i, fat_table_bytes + 512 * i);
  }
  superblock->fat_table = (uint32_t*) fat_table_bytes;

  /*DirectoryEntry* root_directory = (DirectoryEntry*) kmalloc(512);
  ata_device->ReadBlock(ClusterToSector(root_cluster), root_directory);
  printk("root directory legacy name: \"");
  for (int i = 0; i < 11; i++) {
    printk("%c", root_directory->legacy_name[i]);
  }
  printk("\"\n");*/

  /*printk("root directory attributes: 0x%X\n", root_directory->attributes);
  printk("sizeof directory entry should be 0x20: %p\n", sizeof(DirectoryEntry));
  printk("sizeof DirectoryEntryLFN should be 32: %d\n", sizeof(DirectoryEntryLFN));
  printk("\n");*/

  //ReadDir(root_cluster, 0);

  /*uint64_t sectors_per_fat = partition.boot_record.sectors_per_fat;
  printk("fat table first sector:\n");
  for (int i = 0; i < (512 / 32) * 2; i++) {
    if (i % 2 == 0) {
      printk("\n");
    }
    uint64_t value = fat_table[i];
    printk("[0x%X]: %p  ", i, value);
  }
  printk("\n");
  printk("media descriptor type mbr: 0x%X\n", mbr.boot_record.bpb.media_descriptor_type);
  printk("media descriptor type partition: 0x%X\n", partition.boot_record.bpb.media_descriptor_type);*/

  superblock->root_inode = (Inode*) kcalloc(sizeof(Inode));
  *superblock->root_inode = Inode(superblock->partition.boot_record.root_cluster, (char*)"", true, superblock);

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
  return mbr.partition_entries[0].lba_first_sector + partition.boot_record.bpb.num_reserved_sectors;
}

uint64_t Superblock::GetDataRegionStartSector() {
  return GetFatsStartSector() + partition.boot_record.sectors_per_fat * partition.boot_record.bpb.num_fats;
}

uint64_t Superblock::ClusterToSector(uint64_t cluster_number) {
  // reserved sectors, then FATs, then data region
  // clusters and sectors are 512 bytes, so ignore the difference
  return GetDataRegionStartSector() + cluster_number - 2;
}

uint8_t* Superblock::ReadCluster(uint64_t cluster) {
  uint8_t* sector = (uint8_t*) kmalloc(512);
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

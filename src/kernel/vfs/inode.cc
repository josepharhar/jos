#include "kernel/vfs/inode.h"

#include "kernel/vfs/superblock.h"
#include "kernel/vfs/file.h"

namespace vfs {

Inode::Inode(uint64_t cluster,
             char* name,
             bool is_directory,
             Superblock* superblock)
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

  File* file = (File*)kmalloc(sizeof(File));
  *file = File(this);
  return file;
}

struct ReadDirReadDirectorySectorArg {
  Inode* inode;
  uint64_t current_cluster;
  DirectoryEntry* directory_sector;
  stdj::Array<Inode*> list;
  bool reading_lfn;
  char lfn_filename[LFN_BUFFER_LENGTH];
  ReadDirCallback callback;
};
static void ReadDirReadDirectorySector(void* void_arg) {
  ReadDirReadDirectorySectorArg* arg = (ReadDirReadDirectorySectorArg*)void_arg;

  for (int i = 0; i < DIRECTORY_ENTRIES_PER_SECTOR; i++) {
    DirectoryEntry* entry = arg->directory_sector + i;

    switch (entry->attributes) {
      case FILE_ATTRIBUTE_LFN: {
        if (!arg->reading_lfn) {
          // start reading long filename
          arg->reading_lfn = 1;
          memset(arg->lfn_filename, 0, LFN_BUFFER_LENGTH);
        }

        DirectoryEntryLFN* lfn = (DirectoryEntryLFN*)entry;

        // only first five bits of index count
        int lfn_index = (lfn->index & 0x1F) - 1;
        lfn_index *= 13;

        for (int i = 0; i < 5; i++) {
          arg->lfn_filename[lfn_index + i] = lfn->name_1[i];
        }
        for (int i = 0; i < 6; i++) {
          arg->lfn_filename[lfn_index + i + 5] = lfn->name_2[i];
        }
        for (int i = 0; i < 2; i++) {
          arg->lfn_filename[lfn_index + i + 5 + 6] = lfn->name_3[i];
        }

        break;
      }

      case FILE_ATTRIBUTE_ARCHIVE:
      case FILE_ATTRIBUTE_DIRECTORY: {
        Inode* new_inode = (Inode*)kcalloc(sizeof(Inode));
        new_inode->is_directory = entry->attributes == FILE_ATTRIBUTE_DIRECTORY;
        new_inode->superblock = arg->inode->superblock;
        new_inode->size = entry->filesize;

        // set filename
        if (arg->reading_lfn) {
          memcpy(new_inode->filename, arg->lfn_filename, LFN_BUFFER_LENGTH);
          arg->reading_lfn = 0;
        } else {
          int filename_chars_written = 0;
          for (int i = 0; i < 8; i++) {
            if (entry->legacy_name[i] != ' ') {
              new_inode->filename[filename_chars_written++] =
                  entry->legacy_name[i];
            }
          }
          // add file extension
          if (entry->legacy_name[8] != ' ') {
            new_inode->filename[filename_chars_written++] = '.';
          }
          for (int i = 0; i < 3; i++) {
            if (entry->legacy_name[8 + i] != ' ') {
              new_inode->filename[filename_chars_written++] =
                  entry->legacy_name[8 + i];
            }
          }
        }

        /*if (!strcmp(new_inode->filename, "..") ||
        !strcmp(new_inode->filename, ".")) {
        }*/

        // set cluster pointer
        new_inode->cluster = (entry->first_cluster_number_high << 16) |
                             entry->first_cluster_number_low;

        arg->list->Add(new_inode);

        break;
      }

      default:
        break;
    }
  }

  kfree(arg->directory_sector);
  arg->current_cluster = superblock->GetNextCluster(arg->current_cluster);

  if (IsValidCluster(arg->current_cluster)) {
    arg->directory_sector = (DirectoryEntry*)kmalloc(512);
    arg->inode->superblock->ReadCluster(arg->current_cluster,
                                        arg->directory_sector,
                                        ReadDirReadDirectorySector, arg);
  } else {
    // done now i guess
    arg->callback(arg->list);
    delete arg;
  }
}
void Inode::ReadDir(ReadDirCallback callback) {
  if (!is_directory || !IsValidCluster(cluster)) {
    callback(stdj::Array<Inode*>());
    return;
  }

  ReadDirReadDirectorySectorArg* arg = new ReadDirReadDirectorySectorArg();
  arg->reading_lfn = false;
  memset(arg->lfn_filename, 0, LFN_BUFFER_LENGTH);
  arg->list = stdj::Array<Inode*>();
  arg->current_cluster = cluster;
  arg->callback = callback;

  DirectoryEntry* directory_sector = (DirectoryEntry*)kmalloc(512);
  superblock->ReadCluster(current_cluster, directory_sector,
                          ReadDirReadDirectorySector, arg);
}

}  // namespace vfs

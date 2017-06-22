// http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
// https://users.csc.calpoly.edu/~bellardo/courses/2174/454/notes/CPE454-Week04-1.pdf

#include "tags.h"

#include "printk.h"

extern uint64_t tags_pointer[];

static uint64_t original_pointer = 0;
static char* pointer = 0;

static uint64_t GetElapsedBytes() {
  return ((uint64_t) pointer) - original_pointer;
}

template<typename T>
static T ReadValue() {
  T* type_pointer = (T*) pointer;
  T value = *type_pointer;

  type_pointer++;
  pointer = (char*) type_pointer;
  
  return value;
}

struct MultibootHeader {
  uint32_t total_bytes;
  uint32_t reserved;
} __attribute__((packed));

struct TagHeader {
  uint32_t type;
  uint32_t size; // in bytes

  uint32_t GetSizeAdjusted() {
    // magic: 9 -> 16
    return (size + 7) & ~7;
  }
} __attribute__((packed));

struct ELFHeader {
  uint32_t num_entries;
  uint32_t entry_size_bytes;
  uint32_t string_table_index;
} __attribute__((packed));

struct ELFEntry {
  uint64_t name;
  uint64_t unknown_1;
  uint64_t memory_offset;
  uint64_t disk_offset;
  uint64_t size;
  uint64_t unknown_5;
  uint64_t unknown_6;
  uint64_t unknown_7;
} __attribute__((packed));

// mmap entire tag size is 160 bytes
// minus 8 bytes for TagHeader is 152 bytes
// if MMapEntry is 24 bytes, 152 mod 24 is 8 bytes leftover for MMapHeader
// this should be 8 bytes long
struct MMapHeader {
  uint32_t type; // this is zero, idk what it actually is
  uint32_t entry_size_bytes;
} __attribute__((packed));

// after looking at the memory, this should be as large as 3 64bit entries, or 3*8 = 24bytes
struct MMapEntry {
  uint64_t address;
  uint64_t length;
  /*uint32_t type;
  uint32_t zero;*/
  uint32_t type; // 1 = available, 2 = reserved
  uint32_t negative_one;
} __attribute__((packed));

struct TagsInfo ReadTags() {
  struct TagsInfo tags_info;
  tags_info.elf_min_address = 0xFFFFFFFFFFFFFFFF;
  tags_info.elf_max_address = 0;

  pointer = (char*) *tags_pointer;
  original_pointer = (uint64_t) pointer;

  struct MultibootHeader multiboot_header = ReadValue<struct MultibootHeader>();
  uint32_t total_bytes = multiboot_header.total_bytes;

  int read_null_terminator = 0;
  char* string_table;
  struct ELFHeader elf_header;
  struct ELFEntry* elf_entry;
  struct ELFEntry* elf_string_table;

  while (!read_null_terminator && GetElapsedBytes() < total_bytes) {
    char* next_pointer = pointer;
    struct TagHeader tag_header = ReadValue<struct TagHeader>();
    next_pointer += tag_header.GetSizeAdjusted();

    // pointer is now pointing to data section of this tag

    /*printk("tag_type: 0x%X\n", tag_header->type);
    printk("tag_size_bytes: 0x%X\n", tag_header->size);*/

    switch (tag_header.type) {
      case 0:
        // null terminator, stop reading
        read_null_terminator = 1;
        break;

      case 1:
        // command line string (args?)
        // this is empty
        //printk("cmd line string: \"%s\"\n", pointer);
        break;

      case 2:
        // boot loader name
        //printk("boot loader name: \"%s\"\n", pointer);
        break;
      
      case 3:
        // module
        break;

      case 4:
        // basic meminfo
        break;

      case 5:
        // bootdev
        break;

      case 6: {
        // mmap
        struct MMapHeader mmap_header = ReadValue<struct MMapHeader>();
        // there are 6 MMapEntries   tag size = 160, -8 = 152, -8 = 144
        int num_entries = (tag_header.size - 8 - sizeof(struct MMapHeader)) / sizeof(struct MMapEntry);

        int current_avl_entry = 0;

        struct MMapEntry* mmap_entry_array = (struct MMapEntry*) pointer;
        for (int i = 0; i < num_entries; i++) {
          struct MMapEntry mmap_entry = mmap_entry_array[i];

					if (mmap_entry.type == 1) {
						// available
						if (current_avl_entry == 0) {
							tags_info.region_one_address = mmap_entry.address;
							tags_info.region_one_size = mmap_entry.length;
						} else if (current_avl_entry == 1) {
							tags_info.region_two_address = mmap_entry.address;
							tags_info.region_two_size = mmap_entry.length;
						} else {
							printk("there are more than two available memory regions!\n");
						}
						current_avl_entry++;
					}
        }

        break;
      }

      case 7:
        // VBE
        break;

      case 8:
        // framebuffer
        break;

      case 9:
        // elf sections
        elf_header = ReadValue<struct ELFHeader>();
        /*printk("num_entries: %d\n", elf_header.num_entries);
        printk("entry_size_bytes: %d\n", elf_header.entry_size_bytes);
        printk("string_table_index: %d\n", elf_header.string_table_index);*/
        
        // pointer is now at beginning of ELFEntry array
        elf_entry = elf_string_table = (struct ELFEntry*) pointer;
        elf_string_table += elf_header.string_table_index;

        string_table = (char*) elf_string_table->memory_offset;

        for (int i = 0; i < elf_header.num_entries; i++) {
          // skip null entry
          // TODO this could be done with type instead of index
          if (i == 0) {
            continue;
          }

          struct ELFEntry* current_elf_entry = elf_entry + i;

          int name_offset = current_elf_entry->name;
          //printk("elf section %d name: \"%s\"\n", i, string_table + name_offset);

          uint64_t section_min_address = current_elf_entry->memory_offset;
          uint64_t section_max_address = section_min_address + current_elf_entry->size;

          if (section_min_address < tags_info.elf_min_address) {
            tags_info.elf_min_address = section_min_address;
          }
          if (section_max_address > tags_info.elf_max_address) {
            tags_info.elf_max_address = section_max_address;
          }
        }
        break;

      case 10:
        // APM
        break;

      default:
        break;
    }

    pointer = next_pointer;
  }

  // GetElapsedBytes() should equal total_bytes
  // TODO delet this
  // deleting this causes a compile error wtf
  if (GetElapsedBytes() != total_bytes) {
    printk("GetElapsedBytes() != total_bytes, total_bytes: %d, GetElapsedBytes(): %d\n", total_bytes, GetElapsedBytes());
  }

  return tags_info;
}

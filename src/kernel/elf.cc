#include "elf.h"

#include "string.h"
#include "proc.h"
#include "printk.h"
#include "page.h"

struct ELF64Header {
  // ELFCommonHeader common_header;
  char magic[4];
  uint8_t bitsize;  // 1 = 32, 2 = 64
  uint8_t endian;   // 1 = little, 2 = big
  uint8_t version;
  uint8_t abi;  // 0 for SysV
  uint64_t padding;
  uint16_t exe_type;  // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
  uint16_t isa;
  uint32_t elf_version;
  // start of 64 header
  uint64_t program_entry_position;
  uint64_t program_table_position;
  uint64_t section_table_position;
  uint32_t flags;
  uint16_t header_size;
  uint16_t program_entry_size;
  uint16_t program_entry_num;
  uint16_t section_entry_size;
  uint16_t section_entry_num;
  uint16_t section_name_index;
} __attribute__((packed));
static_assert(sizeof(ELF64Header) == 64);

struct ELF64ProgramHeader {
  uint32_t type;
  uint32_t flags;
  uint64_t file_offset;   // off
  uint64_t load_address;  // vaddr
  uint64_t undefined;
  uint64_t file_size;    // filesz
  uint64_t memory_size;  // memsz
  uint64_t alignment;

} __attribute__((packed));
static_assert(sizeof(ELF64ProgramHeader) == 56);

static uint8_t elf_magic[] = {0x7F, 'E', 'L', 'F'};

ELFInfo ELFGetInfo(uint8_t* file, uint64_t filesize) {
  ELFInfo failure;
  failure.success = false;

  if (filesize < sizeof(ELF64Header)) {
    printk("ELFGetInfo filesize too small: %p\n", filesize);
    return failure;
  }

  ELF64Header* header = (ELF64Header*)file;

  if (memcmp(elf_magic, header->magic, 4)) {
    printk("ELFGetInfo wrong magic number: 0x%X\n", *((uint32_t*)file));
    return failure;
  }
  if (header->bitsize != 2) {
    printk("ELFGetInfo wrong bitsize: %d\n", header->bitsize);
    return failure;
  }
  if (header->endian != 1) {
    printk("ELFGetInfo wrong endianness: %d\n", header->endian);
    return failure;
  }
  if (header->exe_type != 2) {
    printk("ELFGetInfo wrong executable type: %d\n", header->exe_type);
    return failure;
  }
  if (header->isa != 0x3E) {
    printk("ELFGetInfo wrong isa: 0x%X\n", header->isa);
    return failure;
  }

  // need to load .text, .data, and .rodata
  // i think one program header has all this information

  // if (header->program_table_position > filesize) {
  if (header->program_entry_size != sizeof(ELF64ProgramHeader)) {
    printk(
        "ELFGetInfo program entry size not equal to ELF64ProgramHeader: "
        "%d\n",
        header->program_entry_size);
    return failure;
  }

  ELF64ProgramHeader* program_table =
      (ELF64ProgramHeader*)(file + header->program_table_position);
  if (header->program_entry_num != 1) {
    printk("ELFGetInfo wrong number of program table entries: %d\n", header->program_entry_num);
    return failure;
  }

  /*for (int i = 0; i < header->program_entry_num; i++) {
    // allocate virtual space for this header entry and copy it there
    printk("program entry\n");
    printk("type: %d\n", program_table->type);
    printk("flags: %d\n", program_table->flags);
    printk("file_offset: %p\n", program_table->file_offset);
    printk("load_address: %p\n", program_table->load_address);
    printk("file_size: %p\n", program_table->file_size);
    printk("memory_size: %p\n", program_table->memory_size);
  }*/

  ELFInfo return_info;
  return_info.success = true;
  return_info.load_address = program_table->load_address;
  return_info.num_bytes = program_table->memory_size;
  return_info.instruction_pointer = program_table->load_address; // TODO is this correct?
  return_info.file_offset = program_table->file_offset;
  return_info.file_size = program_table->file_size;
  return return_info;
}

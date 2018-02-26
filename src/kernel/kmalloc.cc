#include "kmalloc.h"

#include "page.h"
#include "string.h"
#include "printk.h"

// zero can be used as a null pointer since we are using virtual addresses
// we will never get zero as a valid address
#define NULL 0

#define P0 0
#define P1 1
#define P2 2
#define PPAGES 3

#define P0_SIZE 32
#define P1_SIZE 256
#define P2_SIZE 1024
// requests bigger than P2 will be allocated using contiguous virtual pages

// this goes directly before the pointer returned by kmalloc()
// The size of this struct is the alignment for pointers returned by kmalloc()
// TODO: increase num_pages by using a uint16/uint32
struct KmallocMetadata {
  uint8_t pool : 2;       // 0 = P0, 1 = P1, 2 = P2, 3 = pages
  uint8_t num_pages : 6;  // if pool = 3, this is how many pages were used
} __attribute__((packed));

static void* free_pool_p0 = NULL;
static void* free_pool_p1 = NULL;
static void* free_pool_p2 = NULL;

// allocates a new virtual page and sets up a linked list pool on it
// size must be > 8 and < 4096
static void* AllocateNewPool(uint64_t block_size) {
  uint8_t* new_page = (uint8_t*)page::PageAllocate();
  int num_blocks = PAGE_SIZE_BYTES / block_size;

  for (int i = 0; i < num_blocks; i++) {
    uint8_t* new_block = new_page + (block_size * i);
    uint8_t* next_block = new_block + block_size;

    uint64_t* new_block_64 = (uint64_t*)new_block;
    uint64_t next_block_address = (uint64_t)next_block;
    *new_block_64 = next_block_address;
  }

  // set the last block to null
  uint8_t* last_block = new_page + block_size * (num_blocks - 1);
  uint64_t* last_block_64 = (uint64_t*)last_block;
  *last_block_64 = NULL;

  // return the first block in the new linked list
  return new_page;
}

void* kmalloc(uint64_t size) {
  uint64_t effective_size = size + sizeof(struct KmallocMetadata);
  uint64_t* new_block = NULL;
  struct KmallocMetadata metadata = {0};

  // find a block based on the input size
  if (effective_size <= P0_SIZE) {
    if (!free_pool_p0) {
      // pool is empty, need to allocate a new one
      free_pool_p0 = AllocateNewPool(P0_SIZE);
    }
    new_block = (uint64_t*)free_pool_p0;
    free_pool_p0 = (void*)*new_block;
    metadata.pool = P0;
  } else if (effective_size <= P1_SIZE) {
    if (!free_pool_p1) {
      free_pool_p1 = AllocateNewPool(P1_SIZE);
    }
    new_block = (uint64_t*)free_pool_p1;
    free_pool_p1 = (void*)*new_block;
    metadata.pool = P1;
  } else if (effective_size <= P2_SIZE) {
    if (!free_pool_p2) {
      free_pool_p2 = AllocateNewPool(P2_SIZE);
    }
    new_block = (uint64_t*)free_pool_p2;
    free_pool_p2 = (void*)*new_block;
    metadata.pool = P2;
  } else {
    // use contiguous pages
    int effective_size = size + sizeof(struct KmallocMetadata);
    // effective_size / PAGE_SIZE_BYTES rounded up
    int num_pages = (effective_size + PAGE_SIZE_BYTES - 1) / PAGE_SIZE_BYTES;
    new_block = (uint64_t*)page::PageAllocateContiguous(num_pages);
    metadata.pool = PPAGES;
    metadata.num_pages = num_pages;
  }

  struct KmallocMetadata* metadata_pointer = (struct KmallocMetadata*)new_block;
  *metadata_pointer = metadata;
  metadata_pointer++;
  return metadata_pointer;
}

void kfree(void* address) {
  struct KmallocMetadata* metadata = (struct KmallocMetadata*)address;
  metadata--;
  uint64_t* block = (uint64_t*)metadata;

  // push back onto free list, or free pages
  switch (metadata->pool) {
    case P0:
      *block = (uint64_t)free_pool_p0;
      free_pool_p0 = block;
      break;

    case P1:
      *block = (uint64_t)free_pool_p1;
      free_pool_p1 = block;
      break;

    case P2:
      *block = (uint64_t)free_pool_p2;
      free_pool_p2 = block;
      break;

    case PPAGES:
      page::PageFreeContiguous(block, metadata->num_pages);
      break;
  }
}

void* kcalloc(uint64_t size) {
  void* pointer = kmalloc(size);
  memset(pointer, 0, size);
  return pointer;
}

int ReadWriteTest(void* address, int size) {
  uint8_t* pointer = (uint8_t*)address;
  for (int i = 0; i < size; i++) {
    uint8_t value = i % 256;
    pointer[i] = value;
  }
  for (int i = 0; i < size; i++) {
    uint8_t value = i % 256;
    if (pointer[i] != value) {
      return 1;
    }
  }
  return 0;
}

void TestKmalloc() {
  uint64_t num_bytes_allocated = 0;

  uint64_t pointers[512];

  for (int i = 0; i < 512; i++) {
    uint64_t malloc_size = 20 * i;
    void* new_malloc = kmalloc(malloc_size);
    pointers[i] = (uint64_t)new_malloc;
    if (ReadWriteTest(new_malloc, malloc_size)) {
      printk("failed ReadWriteTest() new_malloc: %p, malloc_size: %q\n",
             new_malloc, malloc_size);
      return;
    }
  }

  for (int i = 0; i < 512; i++) {
    kfree((void*)pointers[i]);
  }

  printk("passed all ReadWriteTests()s\n");
}

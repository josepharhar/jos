#include "frame.h"

#include "printk.h"
#include "string.h"

#define FRAME_SIZE_BYTES 4096
#define REGION_MAX_FRAMES 512
#define REGION_MAX_SIZE_BYTES (REGION_MAX_FRAMES * FRAME_SIZE_BYTES)
// AVL one is 159 frames big
// AVL two is 32,480 frames big

// pointer to head of free frame linked list
static uint64_t* free_frame = (uint64_t*) NULL_FRAME_PTR;
static uint64_t* free_region = (uint64_t*) NULL_FRAME_PTR;

static bool initialized = false;

// rounds up address to align to FRAME_SIZE_BYTES
uint64_t AlignAddressUp(uint64_t address) {
  uint64_t leftover = address % FRAME_SIZE_BYTES;
  if (!leftover) {
    return address;
  } else {
    return address + FRAME_SIZE_BYTES - leftover;
  }
}
// rounds down address to align to FRAME_SIZE_BYTES
uint64_t AlignAddressDown(uint64_t address) {
  return address - (address % FRAME_SIZE_BYTES);
}

static void* ClearFrame(void* frame) {
  memset(frame, 0, FRAME_SIZE_BYTES);
  return frame;
}

// sets up linked list structure in frames
// start_address is address of first frame and beginning of region
// end_address is the first memory address outside of the region
// ...|s___|____|____|e...
static void InitializeFrameRegion(uint64_t start_address, uint64_t end_address) {
  uint64_t num_frames = (end_address - start_address) / FRAME_SIZE_BYTES;
  uint8_t* start_pointer = (uint8_t*) start_address;

  for (int i = 0; i < num_frames; i++) {
    uint8_t* new_frame_pointer = start_pointer + FRAME_SIZE_BYTES * i;
    uint8_t* next_frame_pointer = new_frame_pointer + FRAME_SIZE_BYTES;
    //printk("setting up frame #%d at %p -> %p\n", i, new_frame_pointer, next_frame_pointer);
    *((uint64_t*) new_frame_pointer) = (uint64_t) next_frame_pointer;
  }

  // set last entry in region to NULL
  uint8_t* end_pointer = (uint8_t*) end_address;
  uint8_t* last_frame_pointer = end_pointer - FRAME_SIZE_BYTES;
  *((uint64_t*) last_frame_pointer) = NULL_FRAME;
}


void FrameInit(struct TagsInfo tags_info) {
  // organize free memory space into aligned frame regions

  // set up free frame linked list in AVL one
  uint64_t avl_one_start = AlignAddressUp(tags_info.region_one_address);
  uint64_t avl_one_end = AlignAddressDown(tags_info.region_one_address + tags_info.region_one_size);
  InitializeFrameRegion(avl_one_start, avl_one_end);
  free_frame = (uint64_t*) avl_one_start;

  // set up linked list of free regions in AVL two
  //uint64_t avl_two_start = AlignAddressUp(tags_info.region_two_address);
  uint64_t avl_two_start = AlignAddressUp(tags_info.elf_max_address);
  uint64_t avl_two_end = AlignAddressDown(tags_info.region_two_address + tags_info.region_two_size);
  uint64_t num_regions = (avl_two_end - avl_two_start) / REGION_MAX_SIZE_BYTES;
  // throw away extra space after the last region so we don't have to keep track of how big the regions are
  //uint64_t last_region_size = (avl_two_end - avl_two_start) % REGION_MAX_SIZE_BYTES; // if zero, no extra region
  uint8_t* start_pointer = (uint8_t*) avl_two_start;
  // each of these regions is the max size
  for (int i = 0; i < num_regions; i++) {
    uint8_t* region_start_pointer = start_pointer + REGION_MAX_SIZE_BYTES * i;
    uint8_t* region_end_pointer = region_start_pointer + REGION_MAX_SIZE_BYTES;
    *((uint64_t*) region_start_pointer) = (uint64_t) region_end_pointer;
  }
  // set last region to null
  uint8_t* last_region_pointer = start_pointer + REGION_MAX_SIZE_BYTES * (num_regions - 1);
  *((uint64_t*) last_region_pointer) = NULL_FRAME;
  free_region = (uint64_t*)  avl_two_start;

  initialized = true;
}

void* FrameAllocate() {
  if (!initialized) {
    printk("FrameAllocate() called before FrameInit(), halting...\n");
    while (1) {
      asm volatile ("hlt");
    }
  }

  if (free_frame != NULL_FRAME_PTR) {
    // use this frame
    uint64_t* return_value = free_frame;
    free_frame = (uint64_t*) *return_value;
    *return_value = 0;
    return ClearFrame(return_value);
  }

  // ran out of free frames in this region
  if (free_region == NULL_FRAME_PTR) {
    // ran out of free regions, fail
    printk("RAN OUT OF MEMORY, NO MORE FREE REGIONS. halting.\n");
    while (1) asm volatile ("hlt");
    return NULL_FRAME_PTR;
  }

  // move free_region down the linked list
  uint64_t new_region_start = (uint64_t) free_region;
  free_region = (uint64_t*) *free_region;

  // set up a new region
  // all regions here are REGION_MAX_SIZE_BYTES long
  uint64_t new_region_end = new_region_start + REGION_MAX_SIZE_BYTES;
  // TODO delet these printks
  printk("initializing new frame region\n");
  InitializeFrameRegion(new_region_start, new_region_end);
  printk("finished initializing new frame region\n");

  // return the first frame of the new region
  uint64_t* return_frame = (uint64_t*) new_region_start;

  // set free_frame to second frame of new region
  free_frame = (uint64_t*) *return_frame;

  *return_frame = 0;
  return ClearFrame(return_frame);
}

void FrameFree(void* new_frame) {
  uint64_t* new_frame_pointer = (uint64_t*) new_frame;

  // push frame on head free_frame linked list
  *new_frame_pointer = (uint64_t) free_frame;
  free_frame = new_frame_pointer;
}

void TestFrame() {
  // allocate every frame and test it

  char* frame;
  int i = 0;
  while ((frame = (char*) FrameAllocate()) != NULL_FRAME_PTR) {
    printk("testing frame #%d, %p\n", i, frame);
    *frame = 'a';
    *(frame + FRAME_SIZE_BYTES - 1) = 'z';

    if (*frame != 'a' || *(frame + FRAME_SIZE_BYTES - 1) != 'z') {
      printk("frame test failed\n");
      return;
    }

    i++;
  }

  printk("successfully tested %d frames\n", i + 1);
}

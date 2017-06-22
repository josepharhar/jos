#ifndef FRAME_H_
#define FRAME_H_

#include "tags.h"

// i would use 0 for this, but 0 is a valid physical address
#define NULL_FRAME 0xFFFFFFFFFFFFFFFF
#define NULL_FRAME_PTR ((void*) NULL_FRAME)

void FrameInit(struct TagsInfo tags_info);

// return NULL_FRAME_PTR on failure
void* FrameAllocate();
void FrameFree(void* frame);

// rounds up address to align to FRAME_SIZE_BYTES
uint64_t AlignAddressUp(uint64_t address);
// rounds down address to align to FRAME_SIZE_BYTES
uint64_t AlignAddressDown(uint64_t address);

#endif  // FRAME_H_

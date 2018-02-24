#include "page.h"

#include "frame.h"
#include "asm.h"
#include "printk.h"
#include "kmalloc.h"
#include "string.h"
#include "proc.h"

// flags for demand paging
// used in PageTableEntry.available2
#define PAGE_NOT_ALLOCATED 0 // pointer in this entry is not valid
#define PAGE_ALLOCATED 1 // pointer in this entry is valid

#define DO_NOT_CREATE_ENTRIES 0
#define CREATE_ENTRIES 1

#define USER_ACCESSIBLE true
#define NOT_USER_ACCESSIBLE false

static bool initialized = false;

// from paging initialization in boot.asm
extern uint64_t p4_table[];
extern uint64_t p3_table[];
extern uint64_t p2_table[];

// http://wiki.osdev.org/Page_Fault
struct PageFaultError {
  uint32_t protection_violation:1;
  uint32_t write:1; // 0 = read, 1 = write
  uint32_t user:1; // 0 = kernel mode, 1 = user mode
  uint32_t rsvd:1;
  uint32_t instruction_fetch:1;
  uint32_t reserved:27;
} __attribute__((packed));

struct VirtualAddress {
  uint64_t physical_frame_offset:12;
  uint64_t p1_index:9; // index into p1 table
  uint64_t p2_index:9;
  uint64_t p3_index:9;
  uint64_t p4_index:9;
  uint64_t sign_extended:16;
} __attribute__((packed));
static struct VirtualAddress ToVirtualAddress(uint64_t address) {
  struct VirtualAddress* pointer = (struct VirtualAddress*) &address;
  return *pointer;
}
static uint64_t FromVirtualAddress(struct VirtualAddress virtual_address) {
  uint64_t* pointer = (uint64_t*) &virtual_address;
  return *pointer;
}

// http://os.phil-opp.com/entering-longmode.html#paging
struct PageTableEntry {
  uint64_t present:1;
  uint64_t writable:1;
  uint64_t user_accessible:1;
  uint64_t write_through_cache:1;
  uint64_t disable_cache:1;
  uint64_t accessed:1;
  uint64_t dirty:1;
  uint64_t huge_page:1;
  uint64_t global:1;
  uint64_t available1:3;
  uint64_t address:40; // << 12 to get full address
  uint64_t available2:11; // used for demand paging flags
  uint64_t no_execute:1;
} __attribute__((packed));
static struct PageTableEntry ToPageTableEntry(uint64_t entry) {
  struct PageTableEntry* pointer = (struct PageTableEntry*) &entry;
  return *pointer;
}
static uint64_t FromPageTableEntry(struct PageTableEntry table_entry) {
  uint64_t* pointer = (uint64_t*) &table_entry;
  return *pointer;
}
static uint64_t GetAddress(struct PageTableEntry* table_entry) {
  return table_entry->address << 12;
}
static void SetAddress(struct PageTableEntry* table_entry, void* pointer) {
  uint64_t address = (uint64_t) pointer;
  table_entry->address = address >> 12;
}

// points to next free page in kernel heap
static uint64_t kernel_heap_break = 0;
// points one byte past bottom of next free stack
static uint64_t kernel_stacks_break = 0;

uint64_t* Getcr3() {
  uint64_t cr3_value;
  GET_REGISTER("cr3", cr3_value);
  return (uint64_t*) cr3_value;
}

void Setcr3(uint64_t new_cr3) {
  uint64_t cr3_value = new_cr3;
  SET_REGISTER("cr3", cr3_value);
}

// wrapper for frame allocator that halts when memory runs out
void* FrameAllocateSafe() {
  void* pointer = FrameAllocate();
  uint64_t address = (uint64_t) pointer;
  if (address == NULL_FRAME) {
    printk("Ran out of physical frames for paging, halting\n");
    HALT_LOOP();
  }
  return pointer;
}

// gets the pointer to the p1 entry corresponding to this address
// if create, then allocates entries along the way
/*static struct*/ PageTableEntry* GetP1Entry(uint64_t faulting_address, int create, bool user_accessible = false, bool debug = false) {
  struct VirtualAddress virtual_address = ToVirtualAddress(faulting_address);

  // TODO delet this
  user_accessible = true;

  if (debug) {
    printk("Printing page table debug info for address %p\n", faulting_address);
  }

  struct PageTableEntry* p4_entry = (struct PageTableEntry*) (Getcr3() + virtual_address.p4_index);
  if (create) {
    // make sure p4 entry is valid
    p4_entry->present = 1;
    p4_entry->writable = 1;
    if (user_accessible) {
      p4_entry->user_accessible = 1;
    }
  }
  if (p4_entry->available2 == PAGE_NOT_ALLOCATED) {
    // allocate a new p3
    if (create) {
      SetAddress(p4_entry, FrameAllocateSafe());
      p4_entry->available2 = PAGE_ALLOCATED;
    } else {
      printk("tried to access p1 entry when p3 doesn't exist.\n");
      printk("request address: %p halting\n", faulting_address);
      return 0;
    }
  } else if (p4_entry->available2 == PAGE_ALLOCATED) {
    // p3 already allocated, continue
  } else {
    printk("avl2 flags not recognizable in p4 entry: %p, halting\n", *((uint64_t*) p4_entry));
    return 0;
  }
  if (debug) {
    printk("  p4_entry present: %d, writable: %d, user_accessible: %d, flags: %d\n",
        p4_entry->present, p4_entry->writable, p4_entry->user_accessible, p4_entry->available2);
  }

  struct PageTableEntry* p3_entry = (struct PageTableEntry*) GetAddress(p4_entry);
  p3_entry += virtual_address.p3_index;
  if (create) {
    // make sure p3 entry is valid
    p3_entry->present = 1;
    p3_entry->writable = 1;
    if (user_accessible) {
      p3_entry->user_accessible = 1;
    }
  }
  if (p3_entry->available2 == PAGE_NOT_ALLOCATED) {
    // allocate a new p2
    if (create) {
      SetAddress(p3_entry, FrameAllocateSafe());
      p3_entry->available2 = PAGE_ALLOCATED;
    } else {
      printk("tried to access p1 entry when p2 doesn't exist.\n");
      printk("request address: %p halting\n", faulting_address);
      return 0;
    }
  } else if (p3_entry->available2 == PAGE_ALLOCATED) {
    // p2 already allocated, continue
  } else {
    printk("avl2 flags not recognizable in p3 entry: %p, halting\n", *((uint64_t*) p3_entry));
    return 0;
  }
  if (debug) {
    printk("  p3_entry present: %d, writable: %d, user_accessible: %d, flags: %d\n",
        p3_entry->present, p3_entry->writable, p3_entry->user_accessible, p3_entry->available2);
  }

  struct PageTableEntry* p2_entry = (struct PageTableEntry*) GetAddress(p3_entry);
  p2_entry += virtual_address.p2_index;
  if (create) {
    // make sure p2 entry is valid
    p2_entry->present = 1;
    p2_entry->writable = 1;
    if (user_accessible) {
      p2_entry->user_accessible = 1;
    }
  }
  // this could have big entry bit set, in which case we shouldn't go any deeper
  if (p2_entry->available2 == PAGE_NOT_ALLOCATED) {
    // allocate a p1
    // TODO there should be an option here to allocate a huge_page instead
    if (create) {
      SetAddress(p2_entry, FrameAllocateSafe());
      p2_entry->available2 = PAGE_ALLOCATED;
    } else {
      printk("tried to access p1 entry when p1 doesn't exist.\n");
      printk("request address: %p halting\n", faulting_address);
      return 0;
    }
  } else if (p2_entry->available2 == PAGE_ALLOCATED) {
    // TODO what happens when we are trying to access something in the identity map?
    // we should allocate p2 huge pages in order to accomodate, as long as its within usable memory
    /*if (p2_entry->huge_page) {
      // this entry does not point to a p1 table, we are done
      return;
    }*/
    // p1 already allocated, continue
  } else {
    printk("avl2 flags not recognizable in p2 entry: %p, halting\n", *((uint64_t*) p2_entry));
    return 0;
  }
  if (debug) {
    printk("  p2_entry present: %d, writable: %d, user_accessible: %d, flags: %d\n",
        p2_entry->present, p2_entry->writable, p2_entry->user_accessible, p2_entry->available2);
  }

  struct PageTableEntry* p1_entry = (struct PageTableEntry*) GetAddress(p2_entry);
  p1_entry += virtual_address.p1_index;
  //p1_entry->available2 = PAGE_NOT_ALLOCATED; // no physical frame assigned yet
  if (debug) {
    printk("  p1_entry present: %d, writable: %d, user_accessible: %d, flags: %d\n",
        p1_entry->present, p1_entry->writable, p1_entry->user_accessible, p1_entry->available2);
  }
  if (create) {
    if (user_accessible) {
      p1_entry->user_accessible = 1;
    }
  }
  return p1_entry;
  /*if (p1_entry->available2 == PAGE_NOT_ALLOCATED) {
    // allocate a frame for actual data
    SetAddress(p1_entry, FrameAllocateSafe());
    p1_entry->available2 = PAGE_ALLOCATED;
  } else if (p1_entry->available2 == PAGE_ALLOCATED) {
    // data frame already allocated
  } else {
    printk("flags not recognized, halting\n");
    HALT_LOOP();
  }*/
}

static void CheckInitialized() {
  if (!initialized) {
    printk("Paging function used before PageInit(), halting...\n");
    while (1) {
      asm volatile ("hlt");
    }
  }
}

void PageInit() {
  // set kernel heap break
  struct VirtualAddress kernel_heap_break_struct = {0};
  kernel_heap_break_struct.p4_index = P4_KERNEL_HEAP;
  kernel_heap_break = FromVirtualAddress(kernel_heap_break_struct);

  // set kernel stacks break
  struct VirtualAddress kernel_stacks_break_struct = {0};
  kernel_stacks_break_struct.p4_index = P4_KERNEL_STACKS + 1; // +1 to go past end of stacks space
  kernel_stacks_break = FromVirtualAddress(kernel_stacks_break_struct);

  initialized = true;
}

// operates on the kernel heap
// allocates consecutives pages in the virtual address space
void* PageAllocateContiguous(int num_pages) {
  CheckInitialized();

  if (kernel_heap_break + num_pages * PAGE_SIZE_BYTES >= kernel_stacks_break) {
    // not enough virtual address space left in heap
    return 0;
  }

  uint64_t new_pages_start_address = kernel_heap_break;
  for (int i = 0; i < num_pages; i++) {
    PageAllocate();
  }
  return (void*) new_pages_start_address;
}

void* PageAllocate() {
  CheckInitialized();

  if (kernel_heap_break + PAGE_SIZE_BYTES >= kernel_stacks_break) {
    // not enough virtual address space left in heap
    return 0;
  }

  // use virtual address kernel_heap_break and increment it
  uint64_t new_page_address = kernel_heap_break;
  kernel_heap_break += PAGE_SIZE_BYTES;

  GetP1Entry(new_page_address, CREATE_ENTRIES);
  return (void*) new_page_address;
}

// operates on the kernel heap
void PageFreeContiguous(void* page, int num_pages) {
  CheckInitialized();

  uint8_t* pointer = (uint8_t*) page;
  for (int i = 0; i < num_pages; i++) {
    PageFree(pointer + PAGE_SIZE_BYTES * i);
  }
}

// operates on the kernel heap
void PageFree(void* page) {
  CheckInitialized();

  // free the underlying frame
  // mark page as invalid in table
  struct PageTableEntry* p1_entry = GetP1Entry((uint64_t) page, DO_NOT_CREATE_ENTRIES);
  if (!p1_entry->present || !p1_entry->writable || p1_entry->available2 != PAGE_ALLOCATED) {
    // p1_entry should be pointing to a valid frame if PageFree() was called on it
    printk("tried to free page, but p1 entry is not valid: 0x%X halting\n", *((uint64_t*) p1_entry));
    HALT_LOOP();
  }
  p1_entry->present = 0;
  p1_entry->writable = 0;
  p1_entry->available2 = PAGE_NOT_ALLOCATED;
  uint64_t frame_address = GetAddress(p1_entry);
  void* frame_pointer = (void*) frame_address;
  //printk("PageFree() freeing frame %p\n", frame_pointer);
  FrameFree(frame_pointer);

  // TODO potentially free frames used in this area of the page table?
  // TODO reuse virtual page somehow?
  // TODO flush cached page table?
}

void* StackAllocate() {
  CheckInitialized();

  if (kernel_stacks_break - STACK_SIZE_BYTES <= kernel_heap_break) {
    // ran out of virtual addresses
    printk("StackAllocate() ran out of virtual addresses\n");
    return 0;
  }

  // im worried about technically starting the stack outside of the space,
  // so skipping 16 bytes to keep aligned
  uint64_t new_stack_bottom = kernel_stacks_break - 16;
  kernel_stacks_break -= STACK_SIZE_BYTES;
  for (int i = 0; i < 4096; i++) {
    // TODO use one huge page here instead?
    GetP1Entry(kernel_stacks_break + 4096 * i, CREATE_ENTRIES);
  }

  return (void*) new_stack_bottom;
}

void StackFree(void* stack_pointer) {
  CheckInitialized();

  // any address within the stack is valid, align to 2MB to find the stack
  uint64_t stack = (uint64_t) stack_pointer;
  stack -= stack % STACK_SIZE_BYTES;
  // stack now points to top of the stack to free
  for (int i = 0; i < 4096; i++) {
    uint64_t page = stack + 4096 * i;
    struct PageTableEntry* p1_entry = GetP1Entry((uint64_t) page, DO_NOT_CREATE_ENTRIES);
    if (p1_entry->available2 == PAGE_ALLOCATED) {
      uint64_t frame_address = GetAddress(p1_entry);
      FrameFree((void*) frame_address);
    }
    p1_entry->present = 0;
    p1_entry->writable = 0;
    p1_entry->available2 = PAGE_NOT_ALLOCATED;
  }

  // TODO same issues listed at the end of PageFree()
}

void HandlePageFault(uint64_t error_code, uint64_t faulting_address) {
  // TODO handle more stuff in the identity map, not all of it was set up in the page table already, only first 1GB
  //printk("HandlePageFault() error_code: 0x%llX, faulting_address: 0x%llX\n", error_code, faulting_address);
  VirtualAddress virtual_address = ToVirtualAddress(faulting_address);
  PageFaultError error = *((PageFaultError*) &error_code);

  PageTableEntry* p1_entry = 0;

  if (virtual_address.p4_index >= P4_KERNEL_HEAP && virtual_address.p4_index <= P4_KERNEL_STACKS) {
    p1_entry = GetP1Entry(faulting_address, DO_NOT_CREATE_ENTRIES);
  } else if (virtual_address.p4_index >= P4_USERSPACE_START) {
    // valid because userspace is huge? this is terrible TODO
    // TODO TODO TODO
    //p1_entry = GetP1Entry(faulting_address, DO_NOT_CREATE_ENTRIES);
    p1_entry = GetP1Entry(faulting_address, CREATE_ENTRIES, true);
    if (p1_entry) {
      p1_entry->user_accessible = 1;
    }
  } else {
    // http://wiki.osdev.org/Page_Fault
    printk("page fault outside of reserved space:\n");
    printk("  error_code: %p\n", error_code);
    printk("    error.write: %d, error.user: %d, error.protec_violat: %d\n",
        error.write, error.user, error.protection_violation);
    printk("  faulting_address: %p, p4_index: %d\n", faulting_address, virtual_address.p4_index);
    if (proc::IsRunning()) {
      printk("  rip: %p, pid: %d\n", proc::GetCurrentProc()->rip, proc::GetCurrentProc()->pid);
      printk("  rsp: %p, rbp: %p\n", proc::GetCurrentProc()->rsp, proc::GetCurrentProc()->rbp);
    }
    printk("  halting\n");
    HALT_LOOP();
  }

  if (!p1_entry) {
    printk("page fault couldn't find p1 entry\n");
    printk("  error_code: %p\n", error_code);
    printk("    error.write: %p\n", error.write);
    printk("    error.user: %p\n", error.user);
    printk("    error.protection_violation: %p\n", error.protection_violation);
    printk("  faulting_address: %p\n", faulting_address);
    printk("  virtual_address.p4_index: %d\n", virtual_address.p4_index);
    printk("  pid: %d\n", proc::GetCurrentPid());
    //printk("  rip: %p\n", current_proc->rip);
    printk("  halting\n");
    HALT_LOOP();
    //return;
  }

  SetAddress(p1_entry, FrameAllocateSafe()); // allocate a new frame of actual data to resolve this fault
  p1_entry->present = 1;
  p1_entry->writable = 1;
  p1_entry->available2 = PAGE_ALLOCATED;
  printk("user %p -> phys %p\n", faulting_address, GetAddress(p1_entry));
}

// TODO move this to security.cc
bool IsAddressInUserspace(uint64_t address) {
  VirtualAddress virtual_address = ToVirtualAddress(address);
  return virtual_address.p4_index >= P4_USERSPACE_START;
}

void TestPage() {
  printk("allocating page\n");
  uint64_t* page = (uint64_t*) PageAllocate();
  printk("PageAllocate(): %p\n", page);

  printk("*page = 0x1234\n");
  *page = 0x1234;
  printk("*page: 0x%X\n", *page);

  // THIS DOESNT WORK BECAUSE PAGE TABLE IS CACHED
  /*printk("freeing page\n");
  PageFree(page);
  printk("writing to freed page, this should cause an unrecoverable page fault\n");
  *page = 0x5678;
  printk("this message should never be seen\n");*/
}

int AllocateUserSpace(uint64_t address, uint64_t num_bytes) {
  // TODO move this check to security.cc
  // TODO this check could probably be done better
  // TODO 
  /*if (!IsAddressInUserspace(address) || !IsAddressInUserspace(address + num_bytes)) {
    return 1;
  }*/

  for (uint64_t aligned_address = AlignAddressDown(address);
      aligned_address < address + num_bytes;
      aligned_address += PAGE_SIZE_BYTES) {
    GetP1Entry(aligned_address, CREATE_ENTRIES, USER_ACCESSIBLE);
  }

  return 0;
}

// TODO delet this
void PagePrintTableInfo(uint64_t address) {
  GetP1Entry(address, DO_NOT_CREATE_ENTRIES, USER_ACCESSIBLE, true);
}

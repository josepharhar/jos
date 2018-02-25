#include "page.h"

#include "frame.h"
#include "asm.h"
#include "printk.h"
#include "kmalloc.h"
#include "string.h"
#include "proc.h"

// TODO figure out if we need demand paging later
// flags for demand paging
// used in PageTableEntry.available2
// #define PAGE_NOT_ALLOCATED 0  // pointer in this entry is not valid
// #define PAGE_ALLOCATED 1      // pointer in this entry is valid

#define DO_NOT_CREATE_ENTRIES 0
#define CREATE_ENTRIES 1

#define USER_ACCESSIBLE true
#define NOT_USER_ACCESSIBLE false

namespace page {

static bool IsAddressValid(uint64_t address) {
  return address && address != NULL_FRAME;
}

static bool initialized = false;

// from paging initialization in boot.asm
extern uint64_t p4_table[];
extern uint64_t p3_table[];
extern uint64_t p2_table[];

// http://wiki.osdev.org/Page_Fault
struct PageFaultError {
  uint32_t protection_violation : 1;
  uint32_t write : 1;  // 0 = read, 1 = write
  uint32_t user : 1;   // 0 = kernel mode, 1 = user mode
  uint32_t rsvd : 1;
  uint32_t instruction_fetch : 1;
  uint32_t reserved : 27;
} __attribute__((packed));

struct VirtualAddress {
  uint64_t physical_frame_offset : 12;
  uint64_t p1_index : 9;  // index into p1 table
  uint64_t p2_index : 9;
  uint64_t p3_index : 9;
  uint64_t p4_index : 9;
  uint64_t sign_extended : 16;

  static VirtualAddress FromPointer(uint64_t address) {
    VirtualAddress* pointer = (VirtualAddress*)&address;
    return *pointer;
  }
} __attribute__((packed));

// http://os.phil-opp.com/entering-longmode.html#paging
struct PageTableEntry {
  uint64_t present : 1;
  uint64_t writable : 1;
  uint64_t user_accessible : 1;
  uint64_t write_through_cache : 1;
  uint64_t disable_cache : 1;
  uint64_t accessed : 1;
  uint64_t dirty : 1;
  uint64_t huge_page : 1;
  uint64_t global : 1;
  uint64_t available1 : 3;
  uint64_t address : 40;     // << 12 to get full address
  uint64_t available2 : 11;  // used for demand paging flags
  uint64_t no_execute : 1;

  uint64_t GetAddress() { return address << 12; }
  void SetAddress(void* pointer) { address = ((uint64_t)pointer) >> 12; }

  static PageTableEntry FromPointer(uint64_t entry) {
    PageTableEntry* pointer = (PageTableEntry*)&entry;
    return *pointer;
  }
} __attribute__((packed));

static void AllocatePageTableEntry(PageTableEntry* entry,
                                   bool user_accessible) {
  entry->present = 1;
  entry->writable = 1;
  if (user_accessible) {
    entry->user_accessible = 1;
  }
  entry->SetAddress(FrameAllocateSafe());
  // entry->available2 = PAGE_ALLOCATED;
}

uint64_t GetPhysicalAddress(uint64_t cr3,
                            uint64_t address,
                            GetPhysicalAddressRequestType request,
                            bool debug) {
  VirtualAddress virtual_address = VirtualAddress::FromPointer(address);
  bool user_accessible = request == FULL_ALLOCATION_USER;

  PageTableEntry* p4_entry =
      (PageTableEntry*)(((uint64_t*)cr3) + virtual_address.p4_index);
  if (request == GET_P4) {
    return (uint64_t)p4_entry;
  }

  if (!IsAddressValid(p4_entry->GetAddress())) {
    switch (request) {
      case GET_P1:
      case GET_P2:
      case GET_P3:
      case GET_P4:
      case NO_ALLOCATION:
        if (debug) {
          printk("GetPhysicalAddress() null p4\n");
        }
        return NULL_FRAME;
      case FULL_ALLOCATION_USER:
      case FULL_ALLOCATION_KERNEL:
        AllocatePageTableEntry(p4_entry, user_accessible);
        break;
    }
  }

  PageTableEntry* p3_entry = (PageTableEntry*)p4_entry->GetAddress();
  p3_entry += virtual_address.p3_index;
  if (request == GET_P3) {
    return (uint64_t)p3_entry;
  }

  if (!IsAddressValid(p3_entry->GetAddress())) {
    switch (request) {
      case GET_P1:
      case GET_P2:
      case GET_P3:
      case GET_P4:
      case NO_ALLOCATION:
        if (debug) {
          printk("GetPhysicalAddress() null p3\n");
        }
        return NULL_FRAME;
      case FULL_ALLOCATION_USER:
      case FULL_ALLOCATION_KERNEL:
        AllocatePageTableEntry(p3_entry, user_accessible);
        break;
    }
  }

  PageTableEntry* p2_entry = (PageTableEntry*)p3_entry->GetAddress();
  p2_entry += virtual_address.p2_index;
  if (request == GET_P2) {
    return (uint64_t)p2_entry;
  }

  if (!IsAddressValid(p2_entry->GetAddress())) {
    switch (request) {
      case GET_P1:
      case GET_P2:
      case GET_P3:
      case GET_P4:
      case NO_ALLOCATION:
        if (debug) {
          printk("GetPhysicalAddress() null p2\n");
        }
        return NULL_FRAME;
      case FULL_ALLOCATION_USER:
      case FULL_ALLOCATION_KERNEL:
        AllocatePageTableEntry(p2_entry, user_accessible);
        break;
    }
  }

  PageTableEntry* p1_entry = (PageTableEntry*)p2_entry->GetAddress();
  p1_entry += virtual_address.p1_index;
  if (request == GET_P1) {
    return (uint64_t)p1_entry;
  }

  if (!IsAddressValid(p1_entry->GetAddress())) {
    switch (request) {
      case GET_P1:
      case GET_P2:
      case GET_P3:
      case GET_P4:
      case NO_ALLOCATION:
        if (debug) {
          printk("GetPhysicalAddress() null p1\n");
        }
        return NULL_FRAME;
      case FULL_ALLOCATION_USER:
      case FULL_ALLOCATION_KERNEL:
        AllocatePageTableEntry(p1_entry, user_accessible);
        break;
    }
  }

  return p1_entry->GetAddress() + virtual_address.physical_frame_offset;
}

static void* CopyPageTableLevel(void* old_table_void, int level) {
  PageTableEntry* new_table = (PageTableEntry*)FrameAllocateSafe();
  PageTableEntry* old_table = (PageTableEntry*)old_table_void;
  for (int i = 0; i < PAGE_SIZE_BYTES / sizeof(PageTableEntry); i++) {
    new_table[i] = old_table[i];

    if (level == 4 && i < P4_USERSPACE_START) {
      // don't copy inner stuff, its the same for the kernel
      continue;
    }

    if (!old_table[i].GetAddress()) {
      // this page table entry doesn't point to anything.
      continue;
    }

    if (level == 1) {
      // copy physical frame
      void* old_frame = (void*)old_table[i].GetAddress();
      void* new_frame = FrameAllocateSafe();
      memcpy(new_frame, old_frame, PAGE_SIZE_BYTES);
      new_table[i].SetAddress(new_frame);
    } else {
      // copy page table level
      new_table[i].SetAddress(
          CopyPageTableLevel((void*)old_table[i].GetAddress(), level - 1));
    }
  }
  return new_table;
}

uint64_t CopyPageTable(uint64_t cr3) {
  return (uint64_t)CopyPageTableLevel((void*)cr3, 4);
}

void DeletePageTable(uint64_t cr3) {
  // TODO
}

// points to next free page in kernel heap
static uint64_t kernel_heap_break = 0;

static void CheckInitialized() {
  if (!initialized) {
    printk("Paging function used before PageInit(), halting...\n");
    while (1) {
      asm volatile("hlt");
    }
  }
}

void Init() {
  // set kernel heap break
  kernel_heap_break = 0;
  VirtualAddress* kernel_heap_break_struct =
      (VirtualAddress*)&kernel_heap_break;
  kernel_heap_break_struct->p4_index = P4_KERNEL_HEAP;

  initialized = true;
}

// operates on the kernel heap
// allocates consecutives pages in the virtual address space
void* PageAllocateContiguous(int num_pages) {
  CheckInitialized();

  /*if (kernel_heap_break + num_pages * PAGE_SIZE_BYTES >= kernel_stacks_break)
  {
    // not enough virtual address space left in heap
    return 0;
  }*/

  uint64_t new_pages_start_address = kernel_heap_break;
  for (int i = 0; i < num_pages; i++) {
    PageAllocate();
  }
  return (void*)new_pages_start_address;
}

void* PageAllocate() {
  CheckInitialized();

  /*if (kernel_heap_break + PAGE_SIZE_BYTES >= kernel_stacks_break) {
    // not enough virtual address space left in heap
    return 0;
  }*/

  // use virtual address kernel_heap_break and increment it
  uint64_t new_page_address = kernel_heap_break;
  kernel_heap_break += PAGE_SIZE_BYTES;

  uint64_t physical_address =
      GetPhysicalAddress(Getcr3(), new_page_address, FULL_ALLOCATION_KERNEL);
  static int asdf = 0;
  if (asdf < 8) {
    asdf++;
    printk("PageAllocate() returning %p -> %p\n", new_page_address, physical_address);
  }
  return (void*)new_page_address;
}

// operates on the kernel heap
void PageFreeContiguous(void* page, int num_pages) {
  CheckInitialized();

  uint8_t* pointer = (uint8_t*)page;
  for (int i = 0; i < num_pages; i++) {
    PageFree(pointer + PAGE_SIZE_BYTES * i);
  }
}

// operates on the kernel heap
void PageFree(void* page) {
  CheckInitialized();

  // free the underlying frame
  // mark page as invalid in table
  PageTableEntry* p1 =
      (PageTableEntry*)GetPhysicalAddress(Getcr3(), (uint64_t)page, GET_P1);
  if (!p1 || !p1->present || !p1->writable ||
      !IsAddressValid(p1->GetAddress())) {
    printk("tried to free page, but p1 entry is not valid: %p halting\n",
           *((uint64_t*)p1));
    while (1) {
      asm volatile("hlt");
    }
  }
  p1->present = 0;
  p1->writable = 0;
  // p1_entry->available2 = PAGE_NOT_ALLOCATED;
  FrameFree((void*)p1->GetAddress());
  p1->SetAddress(0);

  // TODO potentially free frames used in this area of the page table?
  // TODO reuse virtual page somehow?
  // TODO flush cached page table?
}

uint64_t StackAllocate() {
  CheckInitialized();

  uint64_t stack_size = PAGES_PER_STACK * PAGE_SIZE_BYTES;
  uint64_t address = (uint64_t)PageAllocateContiguous(PAGES_PER_STACK);
  uint64_t bottom_of_stack = address + stack_size - 1;

  return bottom_of_stack;
}

void StackFree(uint64_t bottom_of_stack) {
  CheckInitialized();

  uint64_t stack_size = PAGES_PER_STACK * PAGE_SIZE_BYTES;
  uint64_t address = bottom_of_stack - stack_size + 1;
  PageFreeContiguous((void*)address, PAGES_PER_STACK);
}

void HandlePageFault(uint64_t error_code, uint64_t faulting_address) {
  // TODO handle more stuff in the identity map, not all of it was set up in the
  // page table already, only first 1GB
  VirtualAddress virtual_address =
      VirtualAddress::FromPointer(faulting_address);
  PageFaultError error = *((PageFaultError*)&error_code);

  uint64_t cr3 = Getcr3();
  if (proc::IsRunning() && proc::GetCurrentProc()->cr3 != cr3) {
    printk("HandlePageFault() current proc's cr3 doesn't match actual cr3\n");
  }

  uint64_t physical_address = NULL_FRAME;

  if (virtual_address.p4_index >= P4_KERNEL_HEAP &&
      virtual_address.p4_index <= P4_KERNEL_STACKS) {
    physical_address =
        GetPhysicalAddress(cr3, faulting_address, FULL_ALLOCATION_KERNEL, true);

  } else if (virtual_address.p4_index >= P4_USERSPACE_START) {
    // valid because userspace is huge? this is terrible TODO
    // TODO TODO TODO
    physical_address =
        GetPhysicalAddress(cr3, faulting_address, FULL_ALLOCATION_USER, true);

  } else {
    // http://wiki.osdev.org/Page_Fault
    printk("page fault outside of reserved space:\n");
    printk("  error_code: %p\n", error_code);
    printk("    error.write: %d, error.user: %d, error.protec_violat: %d\n",
           error.write, error.user, error.protection_violation);
    printk("  faulting_address: %p, p4_index: %d\n", faulting_address,
           virtual_address.p4_index);
    if (proc::IsRunning()) {
      printk("  rip: %p, pid: %d\n", proc::GetCurrentProc()->rip,
             proc::GetCurrentProc()->pid);
      printk("  rsp: %p, rbp: %p\n", proc::GetCurrentProc()->rsp,
             proc::GetCurrentProc()->rbp);
    }
    printk("  halting\n");
    HALT_LOOP();
  }

  if (!IsAddressValid(physical_address)) {
    printk("HandlePageFault() failed to allocate physaddr for %p\n",
           faulting_address);
    while (1) {
      asm volatile("hlt");
    }
  }
  printk("virt %p -> phys %p, virtp4: %d\n", faulting_address, physical_address,
         virtual_address.p4_index);
}

// TODO move this to security.cc
bool IsAddressInUserspace(uint64_t address) {
  VirtualAddress virtual_address = VirtualAddress::FromPointer(address);
  return virtual_address.p4_index >= P4_USERSPACE_START;
}

void TestPage() {
  printk("allocating page\n");
  uint64_t* page = (uint64_t*)PageAllocate();
  printk("PageAllocate(): %p\n", page);

  printk("*page = 0x1234\n");
  *page = 0x1234;
  printk("*page: 0x%X\n", *page);

  // THIS DOESNT WORK BECAUSE PAGE TABLE IS CACHED
  /*printk("freeing page\n");
  PageFree(page);
  printk("writing to freed page, this should cause an unrecoverable page
  fault\n");
  *page = 0x5678;
  printk("this message should never be seen\n");*/
}

int AllocateUserSpace(uint64_t cr3, uint64_t address, uint64_t num_bytes) {
  // TODO move this check to security.cc
  // TODO this check could probably be done better
  // TODO
  /*if (!IsAddressInUserspace(address) || !IsAddressInUserspace(address +
  num_bytes)) {
    return 1;
  }*/

  for (uint64_t aligned_address = AlignAddressDown(address);
       aligned_address < address + num_bytes;
       aligned_address += PAGE_SIZE_BYTES) {
    GetPhysicalAddress(cr3, aligned_address, FULL_ALLOCATION_USER);
  }

  return 0;
}

}  // namespace page

#include "main.h"

#include "vga.h"
#include "string.h"
#include "printk.h"
#include "keyboard.h"
#include "stdint.h"
#include "irq.h"
#include "tags.h"
#include "frame.h"
#include "page.h"
#include "syscall.h"
#include "proc.h"
#include "kmalloc.h"
#include "kernel/vfs/superblock.h"
#include "kernel/vfs/inode.h"
#include "kernel/vfs/file.h"
#include "kernel/vfs/find_file.h"
#include "syscall_handler.h"
#include "getc_handler.h"
#include "elf.h"
#include "exec_handler.h"
#include "clone_handler.h"
#include "getpid_handler.h"
#include "pit.h"
#include "ipc_handler.h"
#include "semaphore_handler.h"
#include "unistd.h"
#include "getcwd_handler.h"
#include "opendir_handler.h"
#include "kernel/vfs/globals.h"
#include "wait_handler.h"
#include "fcntl_handler.h"

extern uint64_t stack_top[];
extern uint64_t stack_bottom[];
extern uint64_t p2_table[];

static vfs::ATADevice* CreateATADevice() {
  uint16_t bus_base_port = PRIMARY_ATA_PORT;
  uint16_t master = 0;  // TODO
  uint8_t is_master = 1;
  char name[] = "ata_device_name";
  uint8_t irq = 0;  // TODO
  return vfs::ATADevice::Probe(bus_base_port, master, is_master, name, irq);
}

static vfs::Superblock* superblock = 0;
static void SuperblockReady(vfs::Superblock* new_superblock) {
  // this is called by an interrupt handler
  superblock = new_superblock;
}

static void ProcInit(void* arg) {
  printk("Hello from KERNEL Init process\n");

  // TODO this would normally be called with interrupts disabled...
  //   maybe it doesnt matter since there is only one thing going on rn.
  vfs::ATADevice* ata_device = CreateATADevice();
  printk("got ata_device: %p\n", ata_device);
  vfs::Superblock::Create(ata_device, SuperblockReady);
  while (!superblock) {} // TODO is this gross?
  printk("got superblock: %p\n", superblock);
  vfs::SetRootDirectory(superblock->GetRootInode());

  // TODO delet this
  uint64_t new_cr3 = page::CopyPageTable((uint64_t)Getcr3());
  if (page::GetPhysicalAddress((uint64_t)Getcr3(), 100) !=
      page::GetPhysicalAddress(new_cr3, 100)) {
    printk("WTF\n");
    while (1)
      asm volatile("hlt");
  }

  // exec will put this proc into user mode
  printk("kernel init calling exec...\n");
  execv("/user/init", 0);
  printk("Perhaps not.\n");
  while (1) {
    asm volatile("hlt");
  }
}

void KernelMain() {
  VGA_clear();
  printk("KernelMain()\n\n");

  IRQInit();

  // TODO fix serial - why does it take a while to register keyboard input?
  // should i use serial keyboard input instead of the device thing?
  // if i use serial keyboard input then i think it could solve the problem
  // also experiment with serial in GUI to see if anything is different.
  // also if i don't run this and then start using serial output it works even
  // better. :(
  // SerialInit();

  // initialize memory management and dynamic memory allocation
  TagsInfo tags_info = ReadTags();
  FrameInit(tags_info);
  page::Init();

  // get rid of the null frame, it screws everything up.
  FrameAllocate();

  KeyboardInit();

  PitInit();

  InitSyscall();
  proc::Init();

  InitExec();
  GetcInit();
  InitGetpid();
  InitClone();
  InitSemaphore();
  InitGetcwd();
  InitOpendir();
  InitWait();
  InitFcntl();

  /*// TODO error checking needed here
  ATABlockDevice* block_device = CreateATADevice();
  //Superblock* superblock = block_device->GetSuperblock();
  Superblock* superblock = Superblock::Create(block_device);
  Inode* root_directory = superblock->GetRootInode();
  InitExec(root_directory);*/

  ipc::Init();

  proc::CreateKthread(ProcInit, 0);

  printk("calling procrun....\n");
  proc::Run();

  printk("\nKernelMain() ran out of processes to run. calling ProcPrint()\n");
  proc::Print();

  while (1) {
    asm volatile("hlt");
  }
}

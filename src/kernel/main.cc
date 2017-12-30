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
#include "ata_block_device.h"
#include "kmalloc.h"
#include "vfs.h"
#include "io.h"
#include "syscall_handler.h"
#include "io_handler.h"
#include "elf.h"
#include "exec_handler.h"
#include "files.h"
#include "exec.h"
#include "clone_handler.h"
#include "getpid_handler.h"
#include "pit.h"

extern uint64_t stack_top[];
extern uint64_t stack_bottom[];
extern uint64_t p2_table[];
uint64_t esp_holder = 0;
void PrintStackInfo() {
  printk("p2_table:     %p\n", p2_table);
  printk("stack_bottom: %p\n", stack_bottom);
  printk("stack_top:    %p\n", stack_top);
  asm volatile("movq %rsp,esp_holder");
  printk("esp:          %p\n", esp_holder);
}

void ProcKeyboard(void* arg) {
  while (1) {
    char input = KeyboardRead();
    if (input == '\\') {
      printk("\n");
      Proc::Print();
    } else {
      printk("%c", input);
    }
  }
}

ATABlockDevice* CreateATADevice() {
  uint16_t bus_base_port = PRIMARY_ATA_PORT;
  uint16_t master = 0;  // TODO
  uint8_t is_master = 1;
  char name[] = "ata_device_name";
  uint8_t irq = 0;  // TODO
  return ATABlockDevice::Probe(bus_base_port, master, is_master, name, irq);
}

void PrintIndent(int level) {
  for (int i = 0; i < level; i++) {
    printk("  ");
  }
}

/*void PrintDir(Inode* inode, int level) {
  if (inode->IsDirectory()) {
    PrintIndent(level);
    printk("%s/\n", inode->GetName());
    LinkedList<Inode*> sub_inodes = inode->ReadDir();
    Inode* sub_inode = sub_inodes.GetHead();
    while (sub_inode) {
      if (strcmp(sub_inode->GetName(), "..") && strcmp(sub_inode->GetName(), ".")) {
        PrintDir(sub_inode, level + 1);
      }

      Inode* old_sub_inode = sub_inode;
      sub_inode = sub_inodes.GetNextNoLoop(sub_inode);
      kfree(old_sub_inode);
    }
  } else {
    PrintIndent(level);
    printk("%s\n", inode->GetName());
  }
}
void PrintDir(Inode* inode) {
  PrintDir(inode, 0);
}*/

/*void ProcVFS(void* arg) {
  ATABlockDevice* ata_device = CreateATADevice();
  Superblock* superblock = Superblock::Create(ata_device);

  //PrintDir(superblock->GetRootInode());
  char filename[] = "init";
  Inode* inode = FindFile(superblock->GetRootInode(), filename);
  if (inode) {
    printk("inode->GetName(): \"%s\"\n", inode->GetName());
    printk("inode->GetSize(): %lld\n", inode->GetSize());
    File* file = inode->Open();
    char* data = (char*) kmalloc(inode->GetSize());
    memset(data, '_', inode->GetSize());
    file->Read((uint8_t*) data, inode->GetSize());
    //printk("data:\n%s\n", data);
    
    ELFCreateProc((uint8_t*) data, inode->GetSize());

    kfree(data);
    file->Close();
    kfree(file); // TODO do this somewhere else
    kfree(inode);
  } else {
    printk("unable to find file \"%s\"\n", filename);
  }

  Superblock::Destroy(superblock);
  ATABlockDevice::Destroy(ata_device);

  while (1) {
    printk("%c", KeyboardRead());
  }
}*/

void ProcInit(void* arg) {
  printk("Hello from Init process\n");

  // TODO this is awful
  ATABlockDevice* block_device = CreateATADevice();
  Superblock* superblock = Superblock::Create(block_device);
  Inode* root_directory = superblock->GetRootInode();
  InitExec(root_directory);

  // exec will put this proc into user mode
  //Exec("/user/init");
  // TODO make exec use absolute and relative filepaths
  Exec("init");
  printk("Perhaps not.\n");
  while (1) {
    asm volatile ("hlt");
  }
}

void KernelMain() {
  VGA_clear();
  printk("KernelMain()\n\n");

  IRQInit();

  KeyboardInit();

  // TODO fix serial - why does it take a while to register keyboard input?
  // should i use serial keyboard input instead of the device thing?
  // if i use serial keyboard input then i think it could solve the problem
  // also experiment with serial in GUI to see if anything is different.
  // also if i don't run this and then start using serial output it works even
  // better. :(
  // SerialInit();

  TagsInfo tags_info = ReadTags();
  FrameInit(tags_info);
  PageInit();

  PitInit();

  InitSyscall();
  Proc::Init();
  IOInit();

  //InitFork();
  InitGetpid();

  InitClone();

  /*// TODO error checking needed here
  ATABlockDevice* block_device = CreateATADevice();
  //Superblock* superblock = block_device->GetSuperblock();
  Superblock* superblock = Superblock::Create(block_device);
  Inode* root_directory = superblock->GetRootInode();
  InitExec(root_directory);*/

  Proc::CreateKthread(ProcInit, 0);

  printk("calling procrun....\n");
  Proc::Run();

  printk("\nKernelMain() ran out of processes to run. calling ProcPrint()\n");
  Proc::Print();

  while (1) {
    asm volatile("hlt");
  }
}

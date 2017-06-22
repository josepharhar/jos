#include "exec_handler.h"

#include "syscall_handler.h"
#include "page.h"
#include "files.h"
#include "kmalloc.h"
#include "elf.h"
#include "printk.h"
#include "syscall.h"
#include "asm.h"
#include "proc.h"
#include "user.h"
#include "string.h"

static Inode* root_directory = 0;

void PagePrintTableInfo(uint64_t);

// the proc which called this may or may not be in user mode,
// but we will set it to user mode manually
static void HandleSyscallExec(uint64_t interrupt_number, uint64_t param_1, uint64_t param_2, uint64_t param_3) {
  // param_1 is string of filename of target executable
  // TODO sanitize, max string length
  char* filename = (char*) param_1;
  bool success = false;

  // TODO make filename absolute or relative on PATH
  Inode* inode = FindFile(root_directory, filename); // ASD;FLIJASDF;LKASJF;KL
  if (inode) {
    File* file = inode->Open();
    if (file) {
      uint8_t* file_data = (uint8_t*) kmalloc(file->GetSize());
      file->Read(file_data, file->GetSize()); // ;KJASDF;LKJASDF;LKJ

      ELFInfo elf_info = ELFGetInfo(file_data, file->GetSize());
      if (elf_info.success) {
        SaveState(current_proc);

        // TODO create a new page table for this process and recursively free the current one and its frames in user space
        // TODO blow away/sanitize current_proc's registers
        // TODO allocate user stack
        // TODO set to user mode? only if already user or supposed to be user?

        // TODO sanitize/security.cc this

        // allocate user stack
        //printk("allocating user stack from %p to %p\n", USER_STACK_TOP, USER_STACK_BOTTOM);
        AllocateUserSpace(USER_STACK_TOP, USER_STACK_SIZE);
        uint64_t user_stack_bottom = USER_STACK_BOTTOM - 512 - 4096; // TODO
        // put ProcExit() on stack
        // TODO ProcExit() location must come from user executable, not this one
        uint64_t* stack_pointer = (uint64_t*) user_stack_bottom;
        /*printk("putting procexit on stack at %p\n", user_stack_bottom);
        *stack_pointer = (uint64_t) &ProcExit;*/

        // allocate and fill user text/data
        AllocateUserSpace(elf_info.load_address, elf_info.num_bytes);
        memcpy((void*) elf_info.load_address, file_data + elf_info.file_offset,
            elf_info.file_size);
        
        current_proc->rip = elf_info.instruction_pointer;
        //current_proc->rflags |= (3 << 12);
        // TODO
        current_proc->cs = GDT_USER_CS + 3;
        current_proc->ss = GDT_USER_DS + 3;
        //current_proc->rsp = user_stack_bottom;

        //printk("page table debug for rsp:\n");
        //PagePrintTableInfo(current_proc->rsp - 0x100);
        //PagePrintTableInfo(current_proc->rsp + 0x100);
        //PagePrintTableInfo(current_proc->rsp);
        /*printk("page table debug for rip:\n");
        PagePrintTableInfo(current_proc->rip);*/

        RestoreState(current_proc);
        success = true;
      }

      kfree(file_data);
      file->Close();
      kfree(file);
    }
    kfree(inode);
  }

  if (!success) {
    printk("HandleSyscallExec failed to exec file \"%s\"\n", filename);
  }

  /*printk("exec handler returning\n");
  int asdf = 1;
  while (asdf) {}*/
}

void InitExec(Inode* new_root_directory) {
  SetSyscallHandler(SYSCALL_EXEC, HandleSyscallExec);
  root_directory = new_root_directory;
}

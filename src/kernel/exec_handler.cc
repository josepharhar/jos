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
static void HandleSyscallExec(uint64_t interrupt_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  // param_1 is string of filename of target executable
  // TODO sanitize, max string length
  char* filename = (char*)param_1;
  bool success = false;

  // TODO make filename absolute or relative on PATH
  Inode* inode = FindFile(root_directory, filename);
  if (inode) {
    File* file = inode->Open();
    if (file) {
      uint8_t* file_data = (uint8_t*)kmalloc(file->GetSize());
      file->Read(file_data, file->GetSize());

      ELFInfo elf_info = ELFGetInfo(file_data, file->GetSize());
      if (elf_info.success) {
        proc::ExecCurrentProc(elf_info, file_data);
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
}

void InitExec(Inode* new_root_directory) {
  SetSyscallHandler(SYSCALL_EXEC, HandleSyscallExec);
  root_directory = new_root_directory;
}

#include "getc_handler.h"

#include "printk.h"
#include "syscall.h"
#include "syscall_handler.h"
#include "proc.h"
#include "keyboard.h"
#include "page.h"

static void HandleSyscallGetc(uint64_t syscall_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  if (!proc::IsRunning()) {
    printk("HandleSyscallGetc() must be called from a blocking context\n");
  } else {
    // since the calling process will be currently loaded in memory with its page table,
    // we can write directly into its memory
    if (proc::IsKernel() || IsAddressInUserspace(param_1)) {
      /*char input = KeyboardRead();
      char* write_address = (char*) param_1;
      *write_address = input;*/
      SyscallGetcParams* params = (SyscallGetcParams*)param_1;
      KeyboardReadNoNesting(params);
    } else {
      printk("HandleSyscallGetc() user proc gave write address in kernel space: %p\n", param_1);
    }
  }
}

static void HandleSyscallPutc(uint64_t syscall_number,
                              uint64_t param_1,
                              uint64_t param_2,
                              uint64_t param_3) {
  char output = (char) param_1;
  printk("%c", output);
}

void GetcInit() {
  SetSyscallHandler(SYSCALL_GETC, HandleSyscallGetc);
  SetSyscallHandler(SYSCALL_PUTC, HandleSyscallPutc);
}

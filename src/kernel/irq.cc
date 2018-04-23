#include "irq.h"

#include "irq_handlers.h"
#include "asm.h"
#include "printk.h"
#include "vga.h"
#include "page.h"
#include "idt.h"
#include "serial.h"
#include "idt.h"
#include "user.h"
#include "proc.h"
#include "kernel/time.h"
#include "syscall_handler.h"

#define NUM_IRQ_HANDLERS 256 // 256 supported interrupt handlers

#define PIC1    0x20    /* IO base address for master PIC */
#define PIC2    0xA0    /* IO base address for slave PIC */
#define PIC1_COMMAND  PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND  PIC2
#define PIC2_DATA (PIC2+1)

#define ICW1_ICW4 0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL  0x08    /* Level triggered (edge) mode */
#define ICW1_INIT 0x10    /* Initialization - required! */
 
#define ICW4_8086 0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM 0x10    /* Special fully nested (not) */
#define PIC_EOI 0x20

#define NUM_IDT_ENTRIES 0xFF

#define TRAP_GATE 0xF
#define INTERRUPT_GATE 0xE // should disable interrupts when isr is called

struct IRQHandlerTableEntry {
  IRQHandler handler;
  void* arg;
};
static IRQHandlerTableEntry irq_handler_table[NUM_IRQ_HANDLERS];
void IRQHandlerEmpty(uint64_t interrupt_number, void* arg) {}
void IRQHandlerDefault(uint64_t interrupt_number, void* arg) {
  printk("IRQHandlerDefault() interrupt_number: %lld, halting...\n", interrupt_number);
  if (proc::IsRunning()) {
    printk("  pid: %d, rip: %p\n", proc::GetCurrentProc()->pid, proc::GetCurrentProc()->rip);
    printk("  rsp: %p, rbp: %p\n", proc::GetCurrentProc()->rsp, proc::GetCurrentProc()->rbp);
  }
  asm volatile ("hlt");
  //HALT_LOOP();
}

// this goes in the GDT
struct TSSDescriptor {
  uint16_t segment_limit_0_15;
  uint16_t base_address_0_15;
  uint8_t base_address_16_23;

  uint8_t type:4; // has to be 0b1001
  uint8_t zero:1; // should be 0
  uint8_t dpl:2; // should be zero for kernel level?
  uint8_t present:1; // should be 1

  uint8_t segment_limit_16_19:4;
  uint8_t ignored:3; // should be 0
  uint8_t granularity:1; // if set is the limit of the number of pages - ignore

  uint8_t base_address_24_31;
  uint32_t base_address_32_63;
  uint32_t reserved;
} __attribute__((packed));

struct TSS {
  uint32_t reserved1;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved2;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved3;
  uint16_t reserved4;
  uint16_t io_map_base_address;
} __attribute__((packed));

struct IDTEntry {
  uint16_t function_pointer_0_15;
  uint16_t gdt_selector; // byte offset into GDT - use kernel CS, 0x8
  
  uint8_t ist;
  uint8_t type_attr;

  uint16_t function_pointer_16_31;
  uint32_t function_pointer_32_63;
  uint32_t reserved;
} __attribute__((packed));

// tss descriptor will go in gdt64[2]
extern struct TSSDescriptor gdt64[];
extern struct IDTEntry idt_table[NUM_IDT_ENTRIES];

#define IST_SIZE 4096
static uint8_t ist_stacks[IST_SIZE * 7];
static struct TSS tss;

// these descriptors go in the GDT
struct CodeSegmentDescriptor {
  uint16_t segment_limit_0_15;
  uint16_t base_address_0_15;
  uint8_t base_address_16_23;

  uint8_t a:1;
  uint8_t r:1;
  uint8_t c:1;
  uint8_t one:2;
  uint8_t dpl:2;
  uint8_t p:1;

  uint8_t segment_limit_16_19:4;
  uint8_t avl:1;
  uint8_t l:1;
  uint8_t d:1;
  uint8_t g:1;

  uint8_t base_address_24_31;
} __attribute__((packed));
struct DataSegmentDescriptor {
  uint16_t segment_limit_0_15;
  uint16_t base_address_0_15;
  uint8_t base_address_16_23;

  uint8_t a:1;
  uint8_t w:1;
  uint8_t e:1;
  uint8_t zero:1;
  uint8_t one:1;
  uint8_t dpl:2;
  uint8_t p:1;

  uint8_t segment_limit_16_19:4;
  uint8_t avl:1;
  uint8_t reserved:1;
  uint8_t db:1;
  uint8_t g:1;

  uint8_t base_address_24_31;
} __attribute__((packed));

// you can edit gdt in memory and it will be automatically updated, no need to call lgdt again
// in order to check to make sure that the interrupt handler is on a different stack,
// print out the stack pointer or use gdb to check that you are on a different stack

/**
 * http://wiki.osdev.org/PIC
 * arguments:
 * offset1 - vector offset for master PIC
 *           vectors on the master become offset1..offset1+7
 * offset2 - same for slave PIC: offset2..offset2+7
 */
static void PICRemap(int offset1, int offset2) {
  unsigned char a1, a2;
 
  a1 = inb(PIC1_DATA);                      // save masks
  a2 = inb(PIC2_DATA);
 
  outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  io_wait();
  outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();
 
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();
 
  outb(PIC1_DATA, a1);   // restore saved masks.
  outb(PIC2_DATA, a2);
}

extern uint64_t cr2_register[];
extern uint64_t irq_error_code[];

static InterruptContextType prev_interrupt_context = INTERRUPT_CONTEXT_UNINITIALIZED;
static InterruptContextType interrupt_context = INTERRUPT_CONTEXT_UNINITIALIZED;
static uint64_t last_interrupt_number = 0;
static uint64_t last_syscall_num = 0;

InterruptContextType GetInterruptContext() {
  return interrupt_context;
}

static void PreInterrupt(
    InterruptContextType new_interrupt_context,
    uint64_t interrupt_number) {
  // somehow this if isn't catching the case when the timer interrupt interrupts
  // system calls during the part where i turn interrupts off. why????

  if (interrupt_context == INTERRUPT_CONTEXT_SYSCALL) {
    interrupt_context = INTERRUPT_CONTEXT_SYSCALL_INTERRUPTED;
    // we have interrupted a syscall. should context be saved to proc?
    
    // if RestoreState() is not called during the interrupt, then
    //   we will restore state perfectly back into the syscall handler. i think.
    // if RestoreState() is called, then fucking shit dude.
    
    // if we save context to the current proc, then we could restore context
    // into the syscall handler later. i guess theres nothing wrong with this...
    // right? but thats the whole thing i was trying to avoid earlier
    // to make multiprocessing simpler. for this reason, i will not SaveState
    // here.
    
    // somehow, pid1's context's rip is getting stuck in a syscall handler
    // at the hlt() where rescheduling happens at the end of a syscall.
    // this can only happen if we save context while interrupting that part!
  } else {
    interrupt_context = new_interrupt_context;
    proc::SaveStateToCurrentProc();
  }

  last_interrupt_number = interrupt_number;
}

static void PostInterrupt() {
  if (!proc::IsRunning()) {
    return;
  }

  // TODO why does this break exec()????
  //   is it because disk io still uses the old blocking technique?
  /*if (interrupt_context == INTERRUPT_CONTEXT_SYSCALL_INTERRUPTED) {
    interrupt_context = INTERRUPT_CONTEXT_SYSCALL;
  } else {*/
    interrupt_context = INTERRUPT_CONTEXT_PROCESS;
  //}
}

void c_interrupt_handler_2param(uint64_t interrupt_number, uint64_t error_code) {
  PreInterrupt(INTERRUPT_CONTEXT_TWO_PARAM, interrupt_number);

  switch (interrupt_number) {
    case 8: // #DF double fault - error is always zero
    case 10: //  #TS invalid TSS - error is selector that triggered fault
    case 11: //  #NP segment not present - error is selector index of missing segment
    case 12: //  #SS stack exception - error is selector index or zero
    default:
      printk("2 param interrupt %d error_code: 0x%llX\n", interrupt_number, error_code);
      break;

    case 13: //  #GP general protection - error is selector index or zero
      // TODO user code can trigger this fault at any time, dont kys
      printk("interrupt 13 #GP general protection fault error_code: %p\n", error_code);
      {
        int one = 1;
        while (one) {
          //asm volatile ("hlt");
        }
      }
      break;

    case 14: //  #PF page fault - error code is indicating why fault occured. faulting address is in CR2
      // http://wiki.osdev.org/Exceptions#Page_Fault
      //printk("page fault error_code: 0x%llX, CR2: 0x%llX\n", error_code, *cr2_register);
      page::HandlePageFault(error_code, *cr2_register);
      break;
  }

  IRQEndOfInterrupt(interrupt_number);

  PostInterrupt();
}

void c_interrupt_handler(uint64_t interrupt_number) {
  PreInterrupt(INTERRUPT_CONTEXT_ONE_PARAM, interrupt_number);

  // TODO this is hacky, properly register page fault instead
  if (interrupt_number != 14) {
    IRQHandlerTableEntry irq_handler = irq_handler_table[interrupt_number];
    irq_handler.handler(interrupt_number, irq_handler.arg);
  }

  // tables of interrupts:
  // http://wiki.osdev.org/Interrupts#Standard_ISA_IRQs
  // http://wiki.osdev.org/Exceptions
  // TODO remove this switch in favor of IRQSetHandler()
  switch (interrupt_number) {
    case PIC1_OFFSET:
      // programmable interrupt timer interrupt
      time::TimerInterrupt();
      break;

    /*case PIC1_OFFSET + 1:
      printk("keyboard interrupt\n");
      break;*/

    case PIC1_OFFSET + 4:
      // COM1 (serial) interrupt
      printk("interrupt %d calling SerialHandleInterrupt()\n", interrupt_number);
      SerialHandleInterrupt();
      break;

    case 14:
      // page fault using new error code saving method
      page::HandlePageFault(*irq_error_code, *cr2_register);
      break;
  }

  // this should only be called for PIC interrupts? TODO
  IRQEndOfInterrupt(interrupt_number);

  PostInterrupt();
}

void c_syscall_handler(uint64_t interrupt_number,
                       uint64_t error_code,
                       uint64_t syscall_number,
                       uint64_t param_1,
                       uint64_t param_2,
                       uint64_t param_3) {
  PreInterrupt(INTERRUPT_CONTEXT_SYSCALL, interrupt_number);
  last_syscall_num = syscall_number;

  HandleSyscall(syscall_number, param_1, param_2, param_3);

  // TODO why does machine reset when putting this after EndOfInterruptReschedule()??
  //PostInterrupt();

  // TODO why doesn't this work for all interrupt handlers?
  //   probably because interrupt handlers interrupt syscalls nomatter what
  proc::EndOfSyscallReschedule();

  interrupt_context = INTERRUPT_CONTEXT_PROCESS;

  /*proc::ProcContext* proc = proc::GetCurrentProc();
  static bool asdf = false;
  if (proc->pid == 2) {
    asdf = true;
  }
  if (asdf) {
    printk("Restoring pid %d at %p after syscall %d\n", proc->pid, proc->rip, syscall_number);
  }*/
}

uint64_t GetLastInterruptNumber() {
  return last_interrupt_number;
}
uint64_t GetLastSyscallNum() {
  return last_syscall_num;
}

void IRQInit() {
  // IDT
  //uint64_t default_function_pointer = &asm_interrupt_handler;
  uint64_t default_function_pointer = 0;
  struct IDTEntry default_entry = {0};
  default_entry.function_pointer_0_15 = default_function_pointer & 0xFFFF;
  default_entry.gdt_selector = 0x8;
  default_entry.ist = 0; // interrupt stack table offset TODO should this be different by default?
  default_entry.type_attr = (1 << 7) | INTERRUPT_GATE;
  default_entry.function_pointer_16_31 = (default_function_pointer >> 16) & 0xFFFF;
  default_entry.function_pointer_32_63 = (default_function_pointer >> 32) & 0xFFFFFFFF;
  default_entry.reserved = 0;
  for (int i = 0; i < NUM_IDT_ENTRIES; i++) {
    idt_table[i] = default_entry;
  }
  for (int i = 0; i < NUM_IDT_ENTRIES; i++) {
    struct IDTEntry* entry = idt_table + i;
    uint64_t function_pointer = asm_irq_functions[i];
    entry->function_pointer_0_15 = function_pointer & 0xFFFF;
    entry->function_pointer_16_31 = (function_pointer >> 16) & 0xFFFF;
    entry->function_pointer_32_63 = (function_pointer >> 32) & 0xFFFFFFFF;
  }
  // table of interrupts: http://wiki.osdev.org/Exceptions
  (idt_table + 13)->ist = 1; // #GP general protection fault
  (idt_table + 14)->ist = 2; // #PF page fault
  (idt_table + 8)->ist = 3;  // #DF double fault
  //(idt_table + 0x80)->type_attr = (1 << 7) | TRAP_GATE | (3 << 5); // enable interrupts for system calls, allow user mode
  // INTERRUPT_GATE disables interrupts during system calls because that would just make things too hard.
  (idt_table + 0x80)->type_attr = (1 << 7) | INTERRUPT_GATE | (3 << 5); // allow user mode
  //(idt_table + 0x80)->ist = 5;
  load_idt();

  // TSS
  // rsp0/rst0 is the address of the stack to use for system calls when changing from user to kernel mode
  // TODO use a better stack, i also think this *might* not let you do nested syscalls, but it could work tho
  tss.rsp0 = (uint64_t) (ist_stacks + IST_SIZE * 5);
  tss.rsp1 = 0;
  tss.rsp2 = 0;
  tss.ist1 = (uint64_t) (ist_stacks + IST_SIZE);
  tss.ist2 = (uint64_t) (ist_stacks + IST_SIZE * 2);
  tss.ist3 = (uint64_t) (ist_stacks + IST_SIZE * 3);
  tss.ist4 = (uint64_t) (ist_stacks + IST_SIZE * 4);
  tss.ist5 = (uint64_t) (ist_stacks + IST_SIZE * 5);
  tss.ist6 = (uint64_t) (ist_stacks + IST_SIZE * 6);
  tss.ist7 = (uint64_t) (ist_stacks + IST_SIZE * 7);
  struct TSSDescriptor tss_descriptor;
  uint64_t tss_address = (uint64_t) &tss;
  tss_descriptor.segment_limit_0_15 = sizeof(struct TSS) & 0xFFFF; // TODO is this correct?
  tss_descriptor.base_address_0_15 = tss_address & 0xFFFF;
  tss_descriptor.base_address_16_23 = (tss_address >> 16) & 0xFF;
  //tss_descriptor.type = 0xA1;
  tss_descriptor.type = 9;
  tss_descriptor.zero = 0;
  tss_descriptor.dpl = 0;
  tss_descriptor.present = 1;
  tss_descriptor.segment_limit_16_19 = (sizeof(struct TSS) >> 16) & 0xFF; // TODO is this correct?
  tss_descriptor.ignored = 0;
  tss_descriptor.granularity = 0;
  tss_descriptor.base_address_24_31 = (tss_address >> 24) & 0xFF;
  tss_descriptor.base_address_32_63 = (tss_address >> 32) & 0xFFFFFFFF;
  tss_descriptor.reserved = 0;
  gdt64[2] = tss_descriptor;
  load_tss();

  // GDT
  CodeSegmentDescriptor user_cs = {0};
  user_cs.p = 1; // bit 47
  user_cs.dpl = DPL_USER; // bits 45-46
  user_cs.one = 3; // bits 43-44
  DataSegmentDescriptor user_ds = {0};
  user_ds.p = 1; // bit 47
  user_ds.dpl = DPL_USER; // bits 45-46
  user_ds.one = 1; // bit 44
  user_ds.w = 1; // bit 41
  CodeSegmentDescriptor* gdt_cs = (CodeSegmentDescriptor*) gdt64;
  DataSegmentDescriptor* gdt_ds = (DataSegmentDescriptor*) gdt64;
  // setting is done in boot.asm now
  // TODO figure out if i can change it in memory in here instead without calling lgdt again
  /*gdt_cs[GDT_USER_CS] = user_cs;
  gdt_ds[GDT_USER_DS] = user_ds;*/


  // PIC
  PICRemap(PIC1_OFFSET, PIC2_OFFSET);
  IRQSetMask(0); // disable PIC timer interrupts
  IRQClearMask(1); // enable keyboard interrupts
  //IRQSetMask(1); // disable keyboard interrupts (non-serial)
  IRQClearMask(4); // enable COM1 (serial) interrupts
  //IRQClearMask(0); // enables PIC timer interrupts
  //printk("pic mask: %X\n", IRQGetMask());

  // set up interrupt handler table
  for (int i = 0; i < NUM_IRQ_HANDLERS; i++) {
    IRQSetHandler(&IRQHandlerDefault, i, 0);
  }

  // enable interrupts
  sti();
}

// set means that the interrupt will be blocked
void IRQSetMask(uint8_t IRQline) {
  uint16_t port;
  uint8_t value;

  if(IRQline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = inb(port) | (1 << IRQline);
  outb(port, value);   
}

// clear means that the interrupt can go
void IRQClearMask(uint8_t IRQline) {
  uint16_t port;
  uint8_t value;

  if(IRQline < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = inb(port) & ~(1 << IRQline);
  outb(port, value);  
}

// get entire mask of both PIC chips
int IRQGetMask() {
  return (inb(PIC2_DATA) << 8) | inb(PIC1_DATA);
}

void IRQEndOfInterrupt(int irq) {
  // irq must be within PIC's remapped range in order for this to work
  // within 0x20-0x27 and 0x28-0x2F?
  // TODO change literals to use PIC1_OFFSET/PIC2_OFFSET
  if (irq > 0x2F || irq < 0x20) {
    return;
  }

  if (irq > 0x27) {
    // reset slave pic first
    outb(PIC2_COMMAND, PIC_EOI);
  }
  outb(PIC1_COMMAND, PIC_EOI);
}

void IRQSetHandler(IRQHandler handler, uint64_t interrupt_number, void* arg) {
  if (interrupt_number > NUM_IRQ_HANDLERS) {
    printk("IRQSetHandler cannot set an interrupt higher than %q, requested: %q\n", NUM_IRQ_HANDLERS, interrupt_number);
    return;
  }

  IRQHandlerTableEntry new_entry;
  new_entry.handler = handler;
  new_entry.arg = arg;
  irq_handler_table[interrupt_number] = new_entry;
}

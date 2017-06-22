.text
int_handler:
  //movq $0x123abc, 0x0 // this places magic value "0x123abc" at the beginning of memory
  //movq $0x2f592f412f4b2f4f, 0xB8000
  movq $0x123abc, 0xB8000
  iretq
 
.p2align 4
.global idt_table
idt_table:
  .skip 256*16

idtr:
  .short (256*16)-1
  .quad idt_table

// IST - interrupt stack table
// 64 bytes long
ist_table:
  .skip 64
istr:
  .short 64-1
  .quad istr
 
.globl load_idt
.type load_idt, @function
load_idt:
  lidt idtr

  // fill in 49th spot in idt - this can be done in C instead, even after lidt is called?
  /*movq $int_handler, %rax
  mov %ax, idt_table+49*16
  movw $0x8, idt_table+49*16+2 // replace 0x20 with your code section selector - 0x8
  movw $0x8e00, idt_table+49*16+4
  shr $16, %rax
  mov %ax, idt_table+49*16+6
  shr $16, %rax
  mov %rax, idt_table+49*16+8*/

  ret

.globl load_tss
.type load_tss, @function
load_tss:
  // the byte offset into GDT
  mov $0x20, %ax
  ltr %ax
  ret

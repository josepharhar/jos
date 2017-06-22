global irq_1param
global irq_2param
global irq_syscall
extern c_interrupt_handler
extern c_interrupt_handler_2param
extern HandleSyscall

; rdi: interrupt number
; rsi: error code
; rdx: syscall number
; rcx: syscall param 1
; r8:  syscall param 2
; r9:  syscall param 3

irq_1param:
  ; push everything except rdi
  ; rdi already has interrupt number
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push rbp

  cld
  call c_interrupt_handler

  ; pop all registers to restore state
  ; rdi must be last since it was already pushed
  pop rbp
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rdi

  iretq

irq_2param:
  ; push everything except rdi and rsi
  ; rdi already has interrupt number
  ; rsi already has error code
  ; rsi was pushed first, then rdi was pushed
  push rax
  push rbx
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push rbp

  cld
  call c_interrupt_handler_2param

  ; pop all registers to restore state
  ; rsi must be last since it was pushed first
  ; so pop rdi first, then rsi
  pop rbp
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rdi
  pop rsi
  ; empty pop to get rid of error code on stack
  ; rsp = rsp + 8
  ;mov rsp,rsp+8
  add rsp,8

  iretq

global stack_save_state_address
stack_save_state_address:
  dq 0

; this is the same as irq_1param but calls HandleSyscall instead of c_interrupt_handler
irq_syscall:
  ; push everything except rdi
  ; rdi already has interrupt number
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push rbp

  ; stack_save_state_address must be stored as context
  ; so we can nest syscalls
  mov r9, [stack_save_state_address]
  push r9

  mov [stack_save_state_address], rsp

  cld
  call HandleSyscall

  pop r9
  mov [stack_save_state_address], r9

  ; pop all registers to restore state
  ; rdi must be last since it was already pushed
  pop rbp
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rdi

  iretq

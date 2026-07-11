bits 64

section .text

extern irq_handler

%macro PUSH_REGS 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8

    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
%endmacro

%macro POP_REGS 0
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp

    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

%macro IRQ 2

global irq%1

irq%1:

    push 0
    push %2

    PUSH_REGS

    mov rdi, rsp

    call irq_handler

    POP_REGS

    add rsp,16

    iretq

%endmacro

IRQ 0,32
IRQ 1,33
IRQ 2,34
IRQ 3,35
IRQ 4,36
IRQ 5,37
IRQ 6,38
IRQ 7,39

IRQ 8,40
IRQ 9,41
IRQ 10,42
IRQ 11,43
IRQ 12,44
IRQ 13,45
IRQ 14,46
IRQ 15,47

global irq_stub_table

section .rodata

irq_stub_table:

%assign i 0
%rep 16
dq irq%+i
%assign i i+1
%endrep
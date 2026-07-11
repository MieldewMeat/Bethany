bits 64

section .text

extern isr_handler

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

%macro ISR_NOERR 1

global isr%1

isr%1:

    push 0
    push %1

    PUSH_REGS

    mov rdi, rsp

    call isr_handler

    POP_REGS

    add rsp, 16

    iretq

%endmacro

%macro ISR_ERR 1

global isr%1

isr%1:

    push %1

    PUSH_REGS

    mov rdi, rsp

    call isr_handler

    POP_REGS

    add rsp, 16

    iretq

%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

ISR_ERR 8

ISR_NOERR 9

ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29

ISR_ERR 30

ISR_NOERR 31

global isr_stub_table

section .rodata

isr_stub_table:
%assign i 0
%rep 32
    dq isr%+i
%assign i i+1
%endrep
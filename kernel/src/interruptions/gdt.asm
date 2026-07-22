global gdt_flush
gdt_flush:
    lgdt [rdi]

    mov ax, dx
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push rsi
    lea rax, [rel .reload]
    push rax
    o64 retf
.reload:
    ret

global tss_flush
tss_flush:
    mov ax, di
    ltr ax
    ret
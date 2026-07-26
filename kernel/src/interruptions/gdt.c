#include "gdt.h"
#include <stddef.h>

typedef struct __attribute__((packed)){
    uint16_t limit;
    uint64_t base;
}gdtr_t;

static uint64_t gdt[9];
static gdtr_t gdtr;

typedef struct __attribute__((packed)){
    uint32_t reserved0;
    uint64_t rsp0, rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist0, ist1, ist2, ist3, ist4, ist5, ist6;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_t;

static tss_t tss;

extern void gdt_flush(uint64_t gdtr_addr, uint16_t code_sel, uint16_t date_sel);
extern void tss_flush(uint16_t tss_sel);

static void set_desc(size_t i, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt[i] = (limit & 0xFFFF);
    gdt[i] |= (base & 0xFFFFFF) << 16;
    gdt[i] |= (uint64_t)access << 40;
    gdt[i] |= (uint64_t)((limit >> 16) & 0xF) << 48;
    gdt[i] |= (uint64_t)(flags & 0xF) << 52;
    gdt[i] |= (uint64_t)((base >> 24) & 0xFF) << 56;
}

static void set_tss_desc(size_t i, uint64_t base, uint32_t limit){
    gdt[i] = (limit & 0xFFFF)
    | ((base & 0xFFFFFF) << 16)
    | (0x89ULL << 40)
    | ((uint64_t)((limit >> 16) & 0xF) << 48)
    | (((base >> 24) & 0xFFULL) << 56);
    gdt[i+1] = (base >> 32) & 0xFFFFFFFFULL;
}

void gdt_init(void){
    for(size_t i = 0; i < 9; i++) gdt[i] = 0;

    set_desc(5, 0, 0xFFFFF, 0X9A, 0XA);
    set_desc(6, 0, 0xFFFFF, 0X92, 0XC);

    for(size_t i = 0; i < sizeof(tss); i++) ((uint8_t*)&tss)[i] = 0;
    tss.iomap_base = sizeof(tss_t);

    set_tss_desc(7, (uint64_t)&tss, sizeof(tss) - 1);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)gdt;

    gdt_flush((uint64_t)&gdtr, KERNEL_CODE_SELECTOR, KERNEL_DATA_SELECTOR);
    gdt[7] &= ~(1ULL << 41);
    tss_flush(0x38);
}
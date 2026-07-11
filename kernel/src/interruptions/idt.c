#include <stddef.h>

#include "../print_and_stuff/print.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

#define KERNEL_CODE_SELECTOR 0x28

static idt_entry_t idt[256];
static idtr_t idtr;

static void idt_set_gate(uint8_t vector, void *handler, uint8_t flags){
    uint64_t address = (uint64_t)handler;

    idt[vector].offset_low = address & 0xFFFF;
    idt[vector].selector = KERNEL_CODE_SELECTOR;
    idt[vector].ist = 0;
    idt[vector].type_attributes = flags;
    idt[vector].offset_mid = (address >> 16) & 0xFFFF;
    idt[vector].offset_high = (address >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

static void idt_load(void){
    __asm__ volatile(
        "lidt %0"
        :
        : "m"(idtr)
    );
}

extern void* isr_stub_table[];
extern void* irq_stub_table[];

void idt_init(void){
    idtr.base = (uint64_t)idt;
    idtr.limit = sizeof(idt) - 1;

    for(uint8_t i = 0; i < 32; i++)
        idt_set_gate(i, isr_stub_table[i], 0x8E);

    for(uint8_t i = 0; i < 16; i++)
        idt_set_gate(i + 32, irq_stub_table[i], 0x8E);
    
    idt_load();
}
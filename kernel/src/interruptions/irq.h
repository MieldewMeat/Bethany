#pragma once

#include "isr.h"

typedef void (*irq_handler_t)(interrupt_frame_t*);

void irq_register(uint8_t irq, irq_handler_t handler);

void irq_handler(interrupt_frame_t* frame);

extern void* irq_stub_table[];
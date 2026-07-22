#pragma once
#include <stdint.h>

#define KERNEL_CODE_SELECTOR 0x28
#define KERNEL_DATA_SELECTOR 0x30

void gdt_init(void);
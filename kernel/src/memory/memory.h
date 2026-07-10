#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum {
    MEMORY_USABLE,
    MEMORY_RESERVED,
    MEMORY_ACPI_RECLAIMABLE,
    MEMORY_ACPI_NVS,
    MEMORY_BOOTLOADER_RECLAIMABLE,
    MEMORY_KERNEL,
    MEMORY_FRAMEBUFFER,
    MEMORY_BAD_MEMORY,
    MEMORY_UNKNOWN
} memory_type_t;

typedef struct {
    uint64_t base;
    uint64_t length;
    memory_type_t type;
} memory_region_t;

void memory_init(memory_region_t *src, size_t count);

size_t memory_region_count(void);

const memory_region_t *memory_region(size_t index);

uint64_t memory_total(void);

uint64_t memory_amount(memory_type_t type);

uint64_t memory_highest_address(void);

void memory_print(void);
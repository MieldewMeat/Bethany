#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "pmm.h"

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_WRITE_TROUGH (1ULL << 3)
#define PAGE_CACHE_DISABLE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NO_EXECUTE (1ULL << 63)

typedef uint64_t page_entry_t;
typedef page_entry_t page_table_t[512];

uint64_t vmm_read_cr3(void);



void vmm_init(uint64_t hhdm);

void vmm_map(uint64_t virt, uint64_t phys, uint64_t flags);

void vmm_unmap(uint64_t virt);

uint64_t vmm_translate(uint64_t virt);

void vmm_print(void);

void vmm_debug_address(uint64_t addr);

void vmm_walk(uint64_t addr);

bool vmm_is_mapped(uint64_t virt);

bool vmm_alloc_page(uint64_t virt, uint64_t flags);

void vmm_free_page(uint64_t virt);
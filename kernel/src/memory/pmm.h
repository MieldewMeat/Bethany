#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void pmm_init(uint64_t hhdm);

void* pmm_alloc_page(void);

void pmm_free_page(void* page);

void pmm_print(void);
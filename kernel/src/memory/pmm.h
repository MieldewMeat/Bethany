#pragma once
#include <stdint.h>
#include <stddef.h>

void pmm_init(uint64_t hhdm);

void* pmm_alloc_page(void);

void pmm_free_page(void* page);

void pmm_print(void);
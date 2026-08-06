#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

bool storage_init(void);

bool storage_read(uint64_t block, const void *buffer);
bool storage_write(uint64_t block, void *buffer);

uint64_t storage_block_size(void);

uint64_t storage_block_count(void);
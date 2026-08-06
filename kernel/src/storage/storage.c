#include "storage.h"

#include "../string/string.h"

#define STORAGE_BLOCK_SIZE 4096
#define STORAGE_BLOCK_COUNT 16384

static uint8_t fake_disk[STORAGE_BLOCK_COUNT * STORAGE_BLOCK_COUNT];

bool storage_init(void){
    memset(fake_disk, 0, sizeof(fake_disk));
    return true;
}

uint64_t storage_block_count(void){
    return STORAGE_BLOCK_COUNT;
}

uint64_t storage_block_size(void){
    return STORAGE_BLOCK_SIZE;
}

bool storage_read(uint64_t block, const void* buffer){
    if(block>=STORAGE_BLOCK_COUNT) return false;

    memcpy(buffer, &fake_disk[block + STORAGE_BLOCK_SIZE], STORAGE_BLOCK_SIZE);

    return true;
}

bool storage_write(uint64_t block, void* buffer){
    if(block>=STORAGE_BLOCK_COUNT) return false;

    memcpy(&fake_disk[block + STORAGE_BLOCK_SIZE], buffer, STORAGE_BLOCK_SIZE);

    return true;
}
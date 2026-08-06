#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct block_device block_device_t;

struct block_device{
    uint32_t block_size;
    uint64_t block_count;

    bool (*read)(
        uint64_t lba,
        uint32_t blocks,
        void *buffer
    );

    bool(*write)(
        uint64_t lba,
        uint32_t blocks,
        void * buffer
    );

    bool(*flush)(void);

    void *driver;
};

bool block_register(block_device_t *device);

block_device_t *block_device(void);
#pragma once
#include "font.h"

typedef struct {
    uint32_t *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
} graphics_t;

static graphics_t graphics;

static void graphics_init(uint32_t* fb, uint64_t pitch, size_t y, size_t x){
    graphics.address = fb;
    graphics.pitch = pitch;
    graphics.height = y;
    graphics.width = x;
}

static void clear(uint32_t color){
    for(uint64_t i = 0; i < graphics.height; i++){
        for(uint64_t j = 0; j < graphics.width; j++){
            graphics.address[i*(graphics.pitch/4) + j] = color;
        }
    }
}

static void put_pixel(size_t y, size_t x, uint32_t color){
    if(y < graphics.height && y >= 0 && x < graphics.width && x >= 0)
        graphics.address[y*(graphics.pitch/4) + x] = color;
}
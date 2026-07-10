#include "graphics.h"
#include "font.h"

typedef struct {
    uint32_t *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
} graphics_t;

static graphics_t graphics;

void graphics_init(uint32_t* fb, uint64_t pitch, size_t y, size_t x){
    graphics.address = fb;
    graphics.pitch = pitch;
    graphics.height = y;
    graphics.width = x;
}
void clear(uint32_t color){
    for(uint64_t i = 0; i < graphics.height; i++){
        for(uint64_t j = 0; j < graphics.width; j++){
            graphics.address[i*(graphics.pitch/4) + j] = color;
        }
    }
}

void put_pixel(size_t y, size_t x, uint32_t color){
    if(y < graphics.height && x < graphics.width)
        graphics.address[y*(graphics.pitch/4) + x] = color;
}

uint64_t graphics_width(){
    return graphics.width;
}

uint64_t graphics_height(){
    return graphics.height;
}
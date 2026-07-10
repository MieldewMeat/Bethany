#pragma once
#include <stddef.h>
#include <stdint.h>

void graphics_init(uint32_t* fb, uint64_t pitch, size_t y, size_t x);

void clear(uint32_t color);

void put_pixel(size_t y, size_t x, uint32_t color);

uint64_t graphics_width();

uint64_t graphics_height();
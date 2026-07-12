#pragma once

#include <stddef.h>
#include <stdint.h>

void terminal_move_cursor(int64_t y, int64_t x);
void terminal_set_position(uint64_t y, uint64_t x);

void terminal_clear();

void terminal_init(uint32_t c, uint32_t bgc);

void terminal_putchar(char c);

void terminal_write(const char *str);


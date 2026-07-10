#pragma once

#include <stddef.h>
#include <stdint.h>

void terminal_clear();

void terminal_init(uint32_t c, uint32_t bgc);

void terminal_putchar(char c);

void terminal_write(const char *str);


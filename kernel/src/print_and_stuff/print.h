#pragma once
#include <stdbool.h>
#include <stdint.h>

void print_char(char c);

void print_string(const char* str);

void print_uint(uint64_t val);

void print_hex_ex(uint64_t value, bool prefix);

void print_hex(uint64_t value);
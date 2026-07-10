#pragma once

#include "../video/terminal.h"

void print_char(char c) {
    terminal_putchar(c);
}

void print_string(const char* str) {
    terminal_write(str);
}

void print_uint(uint64_t val) {
    if (val >= 10)
        print_uint(val / 10);

    terminal_putchar('0' + (val % 10));
}

void print_hex_ex(uint64_t value, bool prefix){
    const char hex[] = "0123456789ABCDEF";

    char buffer[17];
    uint8_t index = 0;

    if(prefix) terminal_write("0x");

    if(value == 0){
        terminal_putchar('0');
        return;
    }

    while(value > 0){
        buffer[index++] = hex[value & 0xF];
        value >>= 4;
    }

    while(index > 0) terminal_putchar(buffer[--index]);
}

void print_hex(uint64_t value){
    print_hex_ex(value, 1);
}
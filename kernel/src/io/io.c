#include "io.h"

void outb(uint16_t port, uint8_t value){
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

uint8_t inb(uint16_t port){
    uint8_t value;

    __asm__ volatile(
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

void outl(uint16_t port, uint32_t value){
    __asm__ volatile(
        "outl %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

uint32_t inl(uint16_t port){
    uint32_t value;

    __asm__ volatile(
        "inl %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}
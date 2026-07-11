#include "ps2.h"

#include "../../io/io.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

uint8_t ps2_status(void) {
    return inb(PS2_STATUS);
}

void ps2_wait_read(void) {
    while (!(ps2_status() & 1));
}

void ps2_wait_write(void) {
    while (ps2_status() & 2);
}

uint8_t ps2_read(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

void ps2_write(uint8_t data) {
    ps2_wait_write();
    outb(PS2_DATA, data);
}

void ps2_init(void) {}

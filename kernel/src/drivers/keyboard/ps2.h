#pragma once

#include <stdint.h>

void ps2_init(void);

uint8_t ps2_read(void);
void ps2_write(uint8_t data);

uint8_t ps2_status(void);

void ps2_wait_read(void);
void ps2_wait_write(void);
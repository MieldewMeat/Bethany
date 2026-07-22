#pragma once
#include <stdint.h>
#include <stdbool.h>

extern bool need_reschedule;

void pit_init(uint32_t frequency);

uint64_t pit_ticks(void);
uint64_t pit_seconds(void);
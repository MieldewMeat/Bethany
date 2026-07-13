#include "pit.h"

#include "../interruptions/irq.h"
#include "../print_and_stuff/print.h"
#include "../io/io.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

#define PIT_BASE_FREQUENCY 1193182

bool need_reschedule = false;
static volatile uint64_t ticks = 0;
static volatile uint64_t seconds = 0;

static void pit_irq(interrupt_frame_t* frame){
    (void)frame;

    ticks++;
    if((ticks % 100) == 0) seconds++;

    need_reschedule = true;
}

void pit_init(uint32_t frequency){

    if(frequency == 0) return;


    uint16_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36);

    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    irq_register(0, pit_irq);
}

uint64_t pit_ticks(void){
    return ticks;
}

uint64_t pit_seconds(void){
    return seconds;
}
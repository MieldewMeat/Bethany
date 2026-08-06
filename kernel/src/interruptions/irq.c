#include <stddef.h>

#include "irq.h"
#include "pic.h"

#include "../timer/pit.h"
#include "../print_and_stuff/print.h"
#include "../scheduler/scheduler.h"

static irq_handler_t irq_handlers[16] = {0};

void irq_register(uint8_t irq, irq_handler_t handler){
    if(irq >= 16) return;
    
    irq_handlers[irq] = handler;
}

void irq_handler(interrupt_frame_t* frame){

    uint8_t irq = frame->interrupt - 32;

    if(irq_handlers[irq] != NULL)
        irq_handlers[irq](frame);
    else
        pic_set_mask(irq);
    

    pic_send_eoi(irq);

    if(need_reschedule){
        need_reschedule = false;
        scheduler_schedule();
    }
}
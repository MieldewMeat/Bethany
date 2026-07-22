#include "isr.h"
#include "../print/print.h"

void isr_handler(interrupt_frame_t* frame){
    (void)frame;

    print_string("Interrupt: ");
    print_uint(frame->interrupt);
    
    print_string("\nError code: ");
    print_uint(frame->error);
    print_char('\n');

    for(;;)
        __asm__ volatile("hlt");
}
#include "pic.h"
#include "../io/io.h"

#define PIC1 0x20
#define PIC2 0xA0

#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)

#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10

#define ICW4_8086 0x01

static void io_wait(void){
    outb(0x80, 0);
}

void pic_init(void){
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 32);
    io_wait();
    outb(PIC2_DATA, 40);
    io_wait();

    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0);
    io_wait();
    outb(PIC2_DATA, 0);
}

void pic_send_eoi(uint8_t irq){
    if(irq >= 8)
        outb(PIC2_COMMAND, 0x20);

    outb(PIC1_COMMAND, 0x20);
}
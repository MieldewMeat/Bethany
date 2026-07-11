#include "keyboard.h"
#include "ps2.h"
#include "keyboard_handler.h"

#include "../../interruptions/irq.h"
#include "../../print_and_stuff/print.h"
#include "../../video/terminal.h"

static void keyboard_irq(interrupt_frame_t *frame) {
    (void)frame;

    uint8_t scancode = ps2_read();

    bool extended = scancode == 0xE0;

    if(extended) scancode = ps2_read();

    bool pressed = !(scancode & 0x80);

    update_input(scancode & 0x7F, pressed, extended);

    if(pressed)
        switch(getky_()){
            case KEY_UP: terminal_move_cursor(-1, 0); return;
            case KEY_DOWN: terminal_move_cursor(1, 0); return;
            case KEY_LEFT: terminal_move_cursor(0, -1); return;
            case KEY_RIGHT: terminal_move_cursor(0, 1); return;
            default:
                char c = getch_();
                if(c != 0)
                    if(c != '\b')
                        print_char(c);
                    else
                        print_string("\b \b");
        }
}

void keyboard_init(void) {
    irq_register(1, keyboard_irq);
}
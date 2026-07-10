#include "terminal.h"
#include "graphics.h"
#include "font.h"

typedef struct {
    size_t cursor_x;
    size_t cursor_y;

    uint32_t foreground_color;
    uint32_t background_color;
} terminal_t;

terminal_t terminal;

static void draw_char(size_t height, size_t width, uint8_t c){
    if(c == ' ') return;
    const uint8_t *glyph = font[(uint8_t)c];
    for(uint16_t i = 0; i < 16; i++){
        uint8_t bits = glyph[i];

        for(uint8_t j = 0; j < 8; j++){
            if(bits & (1 << (7 - j))) put_pixel(height+i, width+j, terminal.foreground_color);
            else put_pixel(height+i, width+j, terminal.background_color);
        }
    }
}

void terminal_clear(){
    clear(terminal.background_color);
}

void terminal_init(uint32_t c, uint32_t bgc){
    terminal.foreground_color = c;
    terminal.background_color = bgc;

    terminal.cursor_x = 0;
    terminal.cursor_y = 0;
}

void terminal_putchar(char c){
    switch(c){
        case '\n':
            terminal.cursor_x = 0;
            terminal.cursor_y += FONT_HEIGHT;
            return;
        case '\r':
            terminal.cursor_x = 0;
            return;
        case '\t':
            terminal.cursor_x = ((terminal.cursor_x / FONT_WIDTH + 4) & ~3) * FONT_WIDTH;
            return;
    }
    draw_char(terminal.cursor_y, terminal.cursor_x, c);
    terminal.cursor_x += FONT_WIDTH;
    if(terminal.cursor_x >= graphics_width()){
        terminal.cursor_x = 0;
        terminal.cursor_y += FONT_HEIGHT;
    }
    if(terminal.cursor_y >= graphics_height()){
        terminal_clear();
        terminal.cursor_x = 0;
        terminal.cursor_y = 0;
    }
}

void terminal_write(const char *str){
    for(size_t i = 0; str[i] != '\0'; i++) terminal_putchar(str[i]);
}
#include "terminal.h"
#include "graphics.h"
#include "font.h"

#include "../print_and_stuff/print.h"

typedef struct {
    size_t cursor_x;
    size_t cursor_y;

    uint32_t foreground_color;
    uint32_t background_color;
} terminal_t;

terminal_t terminal;

static void draw_char(size_t height, size_t width, uint8_t c){
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

static inline int64_t floor_div(int64_t a, int64_t b){
    int64_t q = a / b;
    int64_t r = a % b;
    if(r < 0){
        q--;
    }
    return q;
}

static inline int64_t floor_mod(int64_t a, int64_t b){
    int64_t r = a % b;
    if(r < 0){
        r += b;
    }
    return r;
}

void terminal_move_cursor(int64_t y, int64_t x){

    int64_t w = (int64_t)graphics_width();
    int64_t h = (int64_t)graphics_height();

    int64_t new_x = (int64_t)terminal.cursor_x + x * FONT_WIDTH;
    y = y + floor_div(new_x, w);
    terminal.cursor_x = (uint64_t)floor_mod(new_x, w);

    int64_t new_y = (int64_t)terminal.cursor_y + y * FONT_HEIGHT;
    terminal.cursor_y = (uint64_t)floor_mod(new_y, h);
}


void terminal_putchar(char c){
    switch(c){
        case '\n':
            terminal.cursor_x = 0;
            terminal_move_cursor(1,0);
            return;
        case '\r':
            terminal.cursor_x = 0;
            return;
        case '\t':
            int64_t col = terminal.cursor_x / FONT_WIDTH;
            terminal_move_cursor(0, ((col + 4) & ~3) - col);
            return;
        case '\b':
            terminal_move_cursor(0,-1);
            return;
    }
    draw_char(terminal.cursor_y, terminal.cursor_x, c);
    terminal_move_cursor(0,1);
}

void terminal_write(const char *str){
    for(size_t i = 0; str[i] != '\0'; i++) terminal_putchar(str[i]);
}
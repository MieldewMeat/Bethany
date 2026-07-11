#include "keyboard_handler.h"

static keycode_t last_keycode = KEY_NONE;

typedef struct {
    bool lshift;
    bool rshift;
    bool lctrl;
    bool rctrl;
    bool lalt;
    bool ralt;
    bool caps_lock;
} modifier_t;

static modifier_t mod = {0};

static const keycode_t scancode_set1[128] = {
    [0x01] = KEY_ESC,
    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0A] = KEY_9,
    [0x0B] = KEY_0,
    [0x0C] = KEY_MINUS,
    [0x0D] = KEY_EQUAL,
    [0x0E] = KEY_BACKSPACE,
    [0x0F] = KEY_TAB,

    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1A] = KEY_LEFTBRACE,
    [0x1B] = KEY_RIGHTBRACE,
    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_LCTRL,

    [0x1E] = KEY_A,
    [0x1F] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON,
    [0x28] = KEY_APOSTROPHE,
    [0x29] = KEY_GRAVE,
    [0x2A] = KEY_LSHIFT,
    [0x2B] = KEY_BACKSLASH,

    [0x2C] = KEY_Z,
    [0x2D] = KEY_X,
    [0x2E] = KEY_C,
    [0x2F] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_DOT,
    [0x35] = KEY_SLASH,
    [0x36] = KEY_RSHIFT,

    [0x37] = KEY_NONE,
    [0x38] = KEY_LALT,
    [0x39] = KEY_SPACE,
    [0x3A] = KEY_CAPSLOCK,

    [0x3B] = KEY_F1,
    [0x3C] = KEY_F2,
    [0x3D] = KEY_F3,
    [0x3E] = KEY_F4,
    [0x3F] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,

    [0x45] = KEY_NUMLOCK,
    [0x46] = KEY_SCROLLLOCK,

    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
    [0x59] = KEY_NONE,
};

static const keycode_t scancode_set1_extended[128] = {
    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_RCTRL,
    [0x35] = KEY_SLASH,
    [0x38] = KEY_RALT,

    [0x47] = KEY_HOME,
    [0x48] = KEY_UP,
    [0x49] = KEY_PAGEUP,
    [0x4B] = KEY_LEFT,
    [0x4D] = KEY_RIGHT,
    [0x4F] = KEY_END,
    [0x50] = KEY_DOWN,
    [0x51] = KEY_PAGEDOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,
};

static const uint32_t keyboard_map_normal[KEY_COUNT] = {
    [KEY_A] = 'a', [KEY_B] = 'b', [KEY_C] = 'c', [KEY_D] = 'd',
    [KEY_E] = 'e', [KEY_F] = 'f', [KEY_G] = 'g', [KEY_H] = 'h',
    [KEY_I] = 'i', [KEY_J] = 'j', [KEY_K] = 'k', [KEY_L] = 'l',
    [KEY_M] = 'm', [KEY_N] = 'n', [KEY_O] = 'o', [KEY_P] = 'p',
    [KEY_Q] = 'q', [KEY_R] = 'r', [KEY_S] = 's', [KEY_T] = 't',
    [KEY_U] = 'u', [KEY_V] = 'v', [KEY_W] = 'w', [KEY_X] = 'x',
    [KEY_Y] = 'y', [KEY_Z] = 'z',

    [KEY_1] = '1', [KEY_2] = '2', [KEY_3] = '3', [KEY_4] = '4',
    [KEY_5] = '5', [KEY_6] = '6', [KEY_7] = '7', [KEY_8] = '8',
    [KEY_9] = '9', [KEY_0] = '0',

    [KEY_SPACE] = ' ',
    [KEY_ENTER] = '\n',
    [KEY_TAB] = '\t',
    [KEY_BACKSPACE] = '\b',
    [KEY_MINUS] = '\'',
    [KEY_EQUAL] = 0,
    [KEY_COMMA] = ',',
    [KEY_DOT] = '.',
    [KEY_SLASH] = '-',
    [KEY_SEMICOLON] = 'c',
    [KEY_APOSTROPHE] = 0,
    [KEY_GRAVE] = '`',
    [KEY_LEFTBRACE] = '+',
    [KEY_RIGHTBRACE] = 0,
    [KEY_BACKSLASH] = '~',
};

static const uint32_t keyboard_map_shift[KEY_COUNT] = {

    [KEY_A] = 'A', [KEY_B] = 'B', [KEY_C] = 'C', [KEY_D] = 'D',
    [KEY_E] = 'E', [KEY_F] = 'F', [KEY_G] = 'G', [KEY_H] = 'H',
    [KEY_I] = 'I', [KEY_J] = 'J', [KEY_K] = 'K', [KEY_L] = 'L',
    [KEY_M] = 'M', [KEY_N] = 'N', [KEY_O] = 'O', [KEY_P] = 'P',
    [KEY_Q] = 'Q', [KEY_R] = 'R', [KEY_S] = 'S', [KEY_T] = 'T',
    [KEY_U] = 'U', [KEY_V] = 'V', [KEY_W] = 'W', [KEY_X] = 'X',
    [KEY_Y] = 'Y', [KEY_Z] = 'Z',

    [KEY_1] = '!',
    [KEY_2] = '"',
    [KEY_3] = '#',
    [KEY_4] = '$',
    [KEY_5] = '%',
    [KEY_6] = '&',
    [KEY_7] = '/',
    [KEY_8] = '(',
    [KEY_9] = ')',
    [KEY_0] = '=',

    [KEY_MINUS]      = '?',
    [KEY_EQUAL]      = 0,
    [KEY_COMMA]      = ';',
    [KEY_DOT]        = ':',
    [KEY_SLASH]      = '_',
    [KEY_SEMICOLON]  = 'C',
    [KEY_APOSTROPHE] = 0,
    [KEY_GRAVE]      = 0,
    [KEY_LEFTBRACE]  = '*',
    [KEY_RIGHTBRACE] = 0,
    [KEY_BACKSLASH]  = '^',

    [KEY_SPACE]     = ' ',
    [KEY_ENTER]     = '\n',
    [KEY_TAB]       = '\t',
    [KEY_BACKSPACE] = '\b',
};

static const uint32_t keyboard_map_altgr[KEY_COUNT] = {
    [KEY_2] = '@',
    [KEY_7] = '{',
    [KEY_8] = '[',
    [KEY_9] = ']',
    [KEY_0] = '}',
};

static void update_input_state(keycode_t key, bool pressed) {
    switch (key) {
        case KEY_LSHIFT: mod.lshift = pressed; break;
        case KEY_RSHIFT: mod.rshift = pressed; break;
        case KEY_LCTRL:  mod.lctrl  = pressed; break;
        case KEY_RCTRL:  mod.rctrl  = pressed; break;
        case KEY_LALT:   mod.lalt   = pressed; break;
        case KEY_RALT:   mod.ralt   = pressed; break;
        case KEY_CAPSLOCK:
            if (pressed) mod.caps_lock = !mod.caps_lock;
            break;
    }
    if (pressed) last_keycode = key;
}

void update_input(uint8_t key, bool pressed, bool extended) {
    if (extended) {
        if (key < 128) {
            update_input_state(scancode_set1_extended[key], pressed);
        }
    } else {
        if (key < 128) {
            update_input_state(scancode_set1[key], pressed);
        }
    }
}

keycode_t getky_(void) {
    return last_keycode;
}

uint32_t getch_(void){
    keycode_t k = last_keycode;

    switch (k) {
        case KEY_NONE:
        case KEY_LSHIFT:
        case KEY_RSHIFT:
        case KEY_LCTRL:
        case KEY_RCTRL:
        case KEY_LALT:
        case KEY_RALT:
        case KEY_CAPSLOCK: return 0;
    }

    bool shift = mod.lshift || mod.rshift;

    bool shift_table = shift;

    if(mod.caps_lock && k >= KEY_A && k <= KEY_Z){
        shift_table = !shift_table;
    }

    uint32_t c = shift_table ? keyboard_map_shift[k] : keyboard_map_normal[k];

    if((mod.lctrl || mod.rctrl) && c >= 'a' && c <= 'z'){
        c -= 'a' - 1;
    }
    if((mod.lctrl || mod.rctrl) && c >= 'A' && c <= 'Z'){
        c -= 'A' - 1;
    }

    if(mod.lalt || mod.ralt){
        uint32_t alt_code = keyboard_map_altgr[k];
        if(alt_code != 0) c = alt_code;
    }
    return c;
}
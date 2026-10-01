#pragma once
#include <stdint.h>

namespace vga {
enum Color : uint8_t {
    BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GREY,
    DARK_GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED,
    LIGHT_MAGENTA, YELLOW, WHITE
};
void init();
void clear();
void putc(char c);
void set_color(uint8_t fg, uint8_t bg);
}

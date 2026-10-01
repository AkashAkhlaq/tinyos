#include "vga.h"
#include "io.h"

namespace {
constexpr int W = 80, H = 25;
volatile uint16_t* const buffer = reinterpret_cast<volatile uint16_t*>(0xB8000);
int row = 0, col = 0;
uint8_t color = 0x0F;

inline uint16_t cell(char c) { return (uint16_t)(color << 8) | (uint8_t)c; }

void update_cursor() {
    uint16_t pos = row * W + col;
    outb(0x3D4, 0x0F); outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E); outb(0x3D5, (pos >> 8) & 0xFF);
}

void scroll() {
    for (int y = 1; y < H; y++)
        for (int x = 0; x < W; x++)
            buffer[(y - 1) * W + x] = buffer[y * W + x];
    for (int x = 0; x < W; x++) buffer[(H - 1) * W + x] = cell(' ');
    row = H - 1;
}
}  // namespace

namespace vga {

void set_color(uint8_t fg, uint8_t bg) { color = (bg << 4) | (fg & 0x0F); }

void clear() {
    for (int i = 0; i < W * H; i++) buffer[i] = cell(' ');
    row = col = 0;
    update_cursor();
}

void init() {
    set_color(LIGHT_GREY, BLACK);
    outb(0x3D4, 0x0A); outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);   // block-ish cursor
    outb(0x3D4, 0x0B); outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
    clear();
}

void putc(char c) {
    switch (c) {
        case '\n': col = 0; row++; break;
        case '\r': col = 0; break;
        case '\t': col = (col + 4) & ~3; break;
        case '\b':
            if (col > 0) { col--; buffer[row * W + col] = cell(' '); }
            break;
        default:
            buffer[row * W + col] = cell(c);
            col++;
    }
    if (col >= W) { col = 0; row++; }
    if (row >= H) scroll();
    update_cursor();
}

}  // namespace vga

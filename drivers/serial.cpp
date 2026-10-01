#include "serial.h"
#include "io.h"

namespace {
constexpr uint16_t COM1 = 0x3F8;
}

namespace serial {

void init() {
    outb(COM1 + 1, 0x00);   // disable interrupts
    outb(COM1 + 3, 0x80);   // enable DLAB
    outb(COM1 + 0, 0x03);   // 38400 baud
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);   // 8N1
    outb(COM1 + 2, 0xC7);   // FIFO
    outb(COM1 + 4, 0x0B);
}

void write(char c) {
    if (c == '\n') write('\r');
    while (!(inb(COM1 + 5) & 0x20)) {}
    outb(COM1, c);
}

}  // namespace serial

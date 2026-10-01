#include "keyboard.h"
#include "idt.h"
#include "io.h"

namespace {

// US QWERTY, scancode set 1. Index = scancode (0x00 - 0x39).
const char normal_map[] =
    "\0" "\x1b" "1234567890-=" "\b" "\t" "qwertyuiop[]" "\n" "\0"
    "asdfghjkl;'`" "\0" "\\" "zxcvbnm,./" "\0" "*" "\0" " ";
const char shift_map[] =
    "\0" "\x1b" "!@#$%^&*()_+" "\b" "\t" "QWERTYUIOP{}" "\n" "\0"
    "ASDFGHJKL:\"~" "\0" "|" "ZXCVBNM<>?" "\0" "*" "\0" " ";

constexpr int BUF = 256;
volatile char buffer[BUF];
volatile int head = 0, tail = 0;
bool shift = false, caps = false, extended = false;

void push(char c) {
    int next = (head + 1) % BUF;
    if (next != tail) { buffer[head] = c; head = next; }
}

void on_key(Registers*) {
    uint8_t sc = inb(0x60);
    if (sc == 0xE0) { extended = true; return; }
    if (extended) { extended = false; return; }      // ignore arrows etc. for now

    bool released = sc & 0x80;
    sc &= 0x7F;

    if (sc == 0x2A || sc == 0x36) { shift = !released; return; }
    if (released) return;
    if (sc == 0x3A) { caps = !caps; return; }
    if (sc >= sizeof(normal_map) - 1) return;

    char c = shift ? shift_map[sc] : normal_map[sc];
    if (caps && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) c ^= 0x20;
    if (c) push(c);
}

}  // namespace

namespace keyboard {

void init() { idt::register_irq(1, on_key); }

char getchar() {
    for (;;) {
        asm volatile("cli");
        if (head != tail) {
            char c = buffer[tail];
            tail = (tail + 1) % BUF;
            asm volatile("sti");
            return c;
        }
        asm volatile("sti; hlt");                    // wait for the next interrupt
    }
}

}  // namespace keyboard

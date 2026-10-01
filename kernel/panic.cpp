#include "panic.h"
#include "kprintf.h"
#include "vga.h"

void panic(const char* msg) {
    asm volatile("cli");
    vga::set_color(vga::WHITE, vga::RED);
    vga::clear();
    kprintf("\n  *** KERNEL PANIC ***\n\n  %s\n\n  System halted.\n", msg);
    for (;;) asm volatile("hlt");
}

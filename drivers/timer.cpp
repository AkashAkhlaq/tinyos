#include "timer.h"
#include "idt.h"
#include "io.h"

namespace {
volatile uint32_t tick_count = 0;
void on_tick(Registers*) { tick_count = tick_count + 1; }
}

namespace timer {

void init() {
    uint32_t divisor = 1193182 / HZ;
    outb(0x43, 0x36);                       // channel 0, lo/hi byte, square wave
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
    idt::register_irq(0, on_tick);
}

uint32_t ticks() { return tick_count; }

void sleep_ms(uint32_t ms) {
    uint32_t target = tick_count + (ms * HZ + 999) / 1000;
    while (tick_count < target) asm volatile("hlt");
}

}  // namespace timer

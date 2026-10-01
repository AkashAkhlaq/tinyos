#include "heap.h"
#include "idt.h"
#include "keyboard.h"
#include "kprintf.h"
#include "multiboot.h"
#include "panic.h"
#include "serial.h"
#include "shell.h"
#include "timer.h"
#include "vga.h"

extern "C" {
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
extern char kernel_start, kernel_end;
}

static void run_global_constructors() {
    for (auto fn = __init_array_start; fn != __init_array_end; fn++) (*fn)();
}

static void log_ok(const char* what) {
    vga::set_color(vga::LIGHT_GREEN, vga::BLACK);
    kputs("[ OK ] ");
    vga::set_color(vga::LIGHT_GREY, vga::BLACK);
    kprintf("%s\n", what);
}

extern "C" void kernel_main(uint32_t magic, MultibootInfo* mbi) {
    serial::init();
    vga::init();
    run_global_constructors();

    vga::set_color(vga::LIGHT_CYAN, vga::BLACK);
    kputs("TinyOS booting...\n\n");
    vga::set_color(vga::LIGHT_GREY, vga::BLACK);

    if (magic != MULTIBOOT_MAGIC) panic("not booted by a Multiboot-compliant loader");
    bootmem::init(mbi);
    log_ok("Multiboot info parsed");

    gdt::init();      log_ok("GDT loaded");
    pic::remap();
    idt::init();      log_ok("IDT loaded, PIC remapped");
    timer::init();    log_ok("PIT timer running at 100 Hz");
    keyboard::init(); log_ok("PS/2 keyboard driver ready");
    asm volatile("sti");

    // Heap: from the end of the kernel image up to 16 MiB (or end of RAM).
    uintptr_t heap_start = reinterpret_cast<uintptr_t>(&kernel_end) + 0x10000;
    uintptr_t ram_end    = 0x100000 + bootmem::upper_kb() * 1024;
    if (bootmem::upper_kb() == 0) ram_end = heap_start + 0x400000;
    size_t heap_size = ram_end - heap_start;
    if (heap_size > 16 * 1024 * 1024) heap_size = 16 * 1024 * 1024;
    heap::init(heap_start, heap_size);
    log_ok("Kernel heap initialised");

    vga::set_color(vga::LIGHT_CYAN, vga::BLACK);
    kputs("\nTinyOS ready\n");
    vga::set_color(vga::LIGHT_GREY, vga::BLACK);

    shell::run();
    panic("shell returned");
}

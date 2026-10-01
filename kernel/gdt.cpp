#include "idt.h"

namespace {
struct GdtEntry {
    uint16_t limit_low, base_low;
    uint8_t  base_mid, access, granularity, base_high;
} __attribute__((packed));

struct GdtPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

GdtEntry entries[3];
GdtPtr   gdt_ptr;

void set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entries[i].base_low    = base & 0xFFFF;
    entries[i].base_mid    = (base >> 16) & 0xFF;
    entries[i].base_high   = (base >> 24) & 0xFF;
    entries[i].limit_low   = limit & 0xFFFF;
    entries[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    entries[i].access      = access;
}
}  // namespace

namespace gdt {

void init() {
    set(0, 0, 0, 0, 0);                    // null descriptor
    set(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     // kernel code: 0x08
    set(2, 0, 0xFFFFFFFF, 0x92, 0xCF);     // kernel data: 0x10
    gdt_ptr.limit = sizeof(entries) - 1;
    gdt_ptr.base  = reinterpret_cast<uint32_t>(&entries);

    asm volatile(
        "lgdt %0\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "ljmp $0x08, $1f\n\t"
        "1:"
        : : "m"(gdt_ptr) : "eax", "memory");
}

}  // namespace gdt

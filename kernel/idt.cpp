#include "idt.h"
#include "io.h"
#include "kprintf.h"
#include "vga.h"

extern "C" uint32_t isr_stub_table[];

namespace {

struct IdtEntry {
    uint16_t base_low, selector;
    uint8_t  zero, flags;
    uint16_t base_high;
} __attribute__((packed));

struct IdtPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

IdtEntry   table[256];
IdtPtr     idt_ptr;
IrqHandler irq_handlers[16];
uint8_t    pic_mask1 = 0xFF, pic_mask2 = 0xFF;

const char* const exception_names[] = {
    "Divide by zero", "Debug", "Non-maskable interrupt", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present",
    "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating point", "Alignment check", "Machine check", "SIMD floating point",
    "Virtualization", "Control protection", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor injection", "VMM communication", "Security exception", "Reserved"
};

void set_gate(int n, uint32_t handler) {
    table[n].base_low  = handler & 0xFFFF;
    table[n].base_high = (handler >> 16) & 0xFFFF;
    table[n].selector  = 0x08;
    table[n].zero      = 0;
    table[n].flags     = 0x8E;           // present, ring 0, 32-bit interrupt gate
}

[[noreturn]] void exception_screen(Registers* r) {
    asm volatile("cli");
    vga::set_color(vga::WHITE, vga::RED);
    vga::clear();
    kprintf("\n  *** KERNEL PANIC: CPU EXCEPTION ***\n\n");
    kprintf("  %s (int %u, error code 0x%x)\n\n", exception_names[r->int_no], r->int_no, r->err_code);
    kprintf("  EIP=%08x  CS=%04x  EFLAGS=%08x\n", r->eip, r->cs, r->eflags);
    kprintf("  EAX=%08x  EBX=%08x  ECX=%08x  EDX=%08x\n", r->eax, r->ebx, r->ecx, r->edx);
    kprintf("  ESI=%08x  EDI=%08x  EBP=%08x  ESP=%08x\n", r->esi, r->edi, r->ebp, r->esp);
    if (r->int_no == 14) {
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        kprintf("  CR2=%08x (faulting address)\n", cr2);
    }
    kprintf("\n  System halted.\n");
    for (;;) asm volatile("hlt");
}

}  // namespace

extern "C" void isr_handler(Registers* r) {
    if (r->int_no < 32) exception_screen(r);

    int irq = r->int_no - 32;
    if (irq >= 8) outb(0xA0, 0x20);      // EOI to the slave PIC
    outb(0x20, 0x20);                    // EOI to the master PIC
    if (irq_handlers[irq]) irq_handlers[irq](r);
}

namespace pic {

void remap() {
    outb(0x20, 0x11); io_wait();         // start init sequence
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();         // master vectors start at 32
    outb(0xA1, 0x28); io_wait();         // slave vectors start at 40
    outb(0x21, 0x04); io_wait();         // slave on IRQ2
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();         // 8086 mode
    outb(0xA1, 0x01); io_wait();
    outb(0x21, pic_mask1);               // mask everything for now
    outb(0xA1, pic_mask2);
}

void unmask(int irq) {
    if (irq < 8) { pic_mask1 &= ~(1 << irq);        outb(0x21, pic_mask1); }
    else         { pic_mask2 &= ~(1 << (irq - 8));  outb(0xA1, pic_mask2);
                   pic_mask1 &= ~(1 << 2);          outb(0x21, pic_mask1); }  // cascade line
}

}  // namespace pic

namespace idt {

void init() {
    for (int i = 0; i < 48; i++) set_gate(i, isr_stub_table[i]);
    idt_ptr.limit = sizeof(table) - 1;
    idt_ptr.base  = reinterpret_cast<uint32_t>(&table);
    asm volatile("lidt %0" : : "m"(idt_ptr));
}

void register_irq(int irq, IrqHandler handler) {
    irq_handlers[irq] = handler;
    pic::unmask(irq);
}

}  // namespace idt

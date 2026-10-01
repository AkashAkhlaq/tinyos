#pragma once
#include <stdint.h>

struct Registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;   // pusha order
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags;                          // pushed by the CPU
};

using IrqHandler = void (*)(Registers*);

namespace gdt { void init(); }
namespace idt {
void init();
void register_irq(int irq, IrqHandler handler);
}
namespace pic {
void remap();
void unmask(int irq);
}

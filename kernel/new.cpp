// C++ runtime glue: operator new/delete on top of our kernel heap.
#include "heap.h"
#include "panic.h"
#include <stddef.h>

void* operator new(size_t n) {
    void* p = heap::alloc(n);
    if (!p) panic("out of memory in operator new");
    return p;
}
void* operator new[](size_t n) { return operator new(n); }

void operator delete(void* p) noexcept { heap::free(p); }
void operator delete[](void* p) noexcept { heap::free(p); }
void operator delete(void* p, size_t) noexcept { heap::free(p); }
void operator delete[](void* p, size_t) noexcept { heap::free(p); }

extern "C" void __cxa_pure_virtual() { panic("pure virtual function called"); }

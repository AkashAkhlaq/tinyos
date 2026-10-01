#include "multiboot.h"

namespace {
uint32_t lower = 0, upper = 0;
MemRegion regions[bootmem::MAX_REGIONS];
int count = 0;
}

namespace bootmem {

// Copy everything we need out of the bootloader's structures right away,
// before the heap can ever overwrite them.
void init(const MultibootInfo* mbi) {
    if (mbi->flags & (1 << 0)) { lower = mbi->mem_lower; upper = mbi->mem_upper; }
    if (mbi->flags & (1 << 6)) {
        uint32_t addr = mbi->mmap_addr, end = addr + mbi->mmap_length;
        while (addr < end && count < MAX_REGIONS) {
            auto* e = reinterpret_cast<const MultibootMmapEntry*>(addr);
            regions[count++] = { e->base_low, e->base_high, e->len_low, e->len_high, e->type };
            addr += e->size + 4;
        }
    }
}

uint32_t lower_kb() { return lower; }
uint32_t upper_kb() { return upper; }
int region_count() { return count; }
const MemRegion& region(int i) { return regions[i]; }

}  // namespace bootmem

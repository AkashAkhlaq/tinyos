#pragma once
#include <stdint.h>

constexpr uint32_t MULTIBOOT_MAGIC = 0x2BADB002;

struct MultibootInfo {
    uint32_t flags;
    uint32_t mem_lower, mem_upper;   // KiB below 1 MiB / above 1 MiB
    uint32_t boot_device, cmdline, mods_count, mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length, mmap_addr;
} __attribute__((packed));

struct MultibootMmapEntry {
    uint32_t size;
    uint32_t base_low, base_high;
    uint32_t len_low, len_high;
    uint32_t type;                   // 1 = usable RAM
} __attribute__((packed));

struct MemRegion {
    uint32_t base_low, base_high, len_low, len_high, type;
};

namespace bootmem {
constexpr int MAX_REGIONS = 32;
void init(const MultibootInfo* mbi);
uint32_t lower_kb();
uint32_t upper_kb();
int region_count();
const MemRegion& region(int i);
}

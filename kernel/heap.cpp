#include "heap.h"
#include "panic.h"

namespace {

constexpr uint32_t MAGIC = 0xC0FFEE42;

struct Block {
    uint32_t size;      // payload bytes
    uint32_t free;
    Block*   next;
    uint32_t magic;
};                      // 16 bytes, keeps payloads 8-byte aligned

Block*  head = nullptr;
size_t  heap_total = 0;

size_t align8(size_t n) { return (n + 7) & ~size_t(7); }

}  // namespace

namespace heap {

void init(uintptr_t start, size_t size) {
    head = reinterpret_cast<Block*>(start);
    head->size  = size - sizeof(Block);
    head->free  = 1;
    head->next  = nullptr;
    head->magic = MAGIC;
    heap_total  = size;
}

void* alloc(size_t n) {
    if (!n) n = 1;
    n = align8(n);
    for (Block* b = head; b; b = b->next) {
        if (!b->free || b->size < n) continue;
        if (b->size >= n + sizeof(Block) + 8) {          // split
            auto* rest = reinterpret_cast<Block*>(reinterpret_cast<uint8_t*>(b + 1) + n);
            rest->size  = b->size - n - sizeof(Block);
            rest->free  = 1;
            rest->next  = b->next;
            rest->magic = MAGIC;
            b->size = n;
            b->next = rest;
        }
        b->free = 0;
        return b + 1;
    }
    return nullptr;
}

void free(void* p) {
    if (!p) return;
    Block* b = static_cast<Block*>(p) - 1;
    if (b->magic != MAGIC) panic("heap corruption: bad block header in free()");
    if (b->free)           panic("heap corruption: double free");
    b->free = 1;
    for (Block* c = head; c && c->next;) {               // coalesce neighbours
        if (c->free && c->next->free) {
            c->size += sizeof(Block) + c->next->size;
            c->next = c->next->next;
        } else {
            c = c->next;
        }
    }
}

size_t total() { return heap_total; }

size_t used() {
    size_t u = 0;
    for (Block* b = head; b; b = b->next) if (!b->free) u += b->size + sizeof(Block);
    return u;
}

size_t available() { return heap_total - used(); }

}  // namespace heap

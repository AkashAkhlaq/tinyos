#pragma once
#include <stddef.h>
#include <stdint.h>

namespace heap {
void   init(uintptr_t start, size_t size);
void*  alloc(size_t n);
void   free(void* p);
size_t total();
size_t used();
size_t available();
}

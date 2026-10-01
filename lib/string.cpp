#include "kstring.h"
#include <stdint.h>

extern "C" {

void* memcpy(void* dst, const void* src, size_t n) {
    auto* d = static_cast<uint8_t*>(dst);
    auto* s = static_cast<const uint8_t*>(src);
    while (n--) *d++ = *s++;
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    auto* d = static_cast<uint8_t*>(dst);
    auto* s = static_cast<const uint8_t*>(src);
    if (d < s) { while (n--) *d++ = *s++; }
    else       { d += n; s += n; while (n--) *--d = *--s; }
    return dst;
}

void* memset(void* dst, int c, size_t n) {
    auto* d = static_cast<uint8_t*>(dst);
    while (n--) *d++ = static_cast<uint8_t>(c);
    return dst;
}

int memcmp(const void* a, const void* b, size_t n) {
    auto* x = static_cast<const uint8_t*>(a);
    auto* y = static_cast<const uint8_t*>(b);
    for (; n; n--, x++, y++)
        if (*x != *y) return *x - *y;
    return 0;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return static_cast<uint8_t>(*a) - static_cast<uint8_t>(*b);
}

int strncmp(const char* a, const char* b, size_t n) {
    for (; n; n--, a++, b++) {
        if (*a != *b) return static_cast<uint8_t>(*a) - static_cast<uint8_t>(*b);
        if (!*a) break;
    }
    return 0;
}

}  // extern "C"

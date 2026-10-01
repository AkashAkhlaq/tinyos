#include "kprintf.h"
#include "kstring.h"
#include "serial.h"
#include "vga.h"
#include <stdint.h>

void kputc(char c) {
    vga::putc(c);
    serial::write(c);
}

void kputs(const char* s) {
    while (*s) kputc(*s++);
}

static void print_number(uint32_t v, unsigned base, bool negative,
                         int width, bool zero_pad, bool upper) {
    char digits[34];
    int n = 0;
    if (v == 0) digits[n++] = '0';
    while (v) {
        unsigned d = v % base;
        digits[n++] = d < 10 ? '0' + d : (upper ? 'A' : 'a') + d - 10;
        v /= base;
    }
    int len = n + (negative ? 1 : 0);
    if (!zero_pad) for (int i = len; i < width; i++) kputc(' ');
    if (negative) kputc('-');
    if (zero_pad)  for (int i = len; i < width; i++) kputc('0');
    while (n) kputc(digits[--n]);
}

void kvprintf(const char* fmt, va_list ap) {
    for (; *fmt; fmt++) {
        if (*fmt != '%') { kputc(*fmt); continue; }
        fmt++;
        bool zero = false;
        int width = 0;
        if (*fmt == '0') { zero = true; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
        switch (*fmt) {
            case 'd': case 'i': {
                int32_t v = va_arg(ap, int32_t);
                print_number(v < 0 ? 0u - (uint32_t)v : (uint32_t)v, 10, v < 0, width, zero, false);
                break;
            }
            case 'u': print_number(va_arg(ap, uint32_t), 10, false, width, zero, false); break;
            case 'x': print_number(va_arg(ap, uint32_t), 16, false, width, zero, false); break;
            case 'X': print_number(va_arg(ap, uint32_t), 16, false, width, zero, true);  break;
            case 'p': kputs("0x"); print_number((uint32_t)va_arg(ap, void*), 16, false, 8, true, false); break;
            case 'c': kputc((char)va_arg(ap, int)); break;
            case 's': { const char* s = va_arg(ap, const char*); kputs(s ? s : "(null)"); break; }
            case '%': kputc('%'); break;
            case '\0': return;
            default:  kputc('%'); kputc(*fmt); break;
        }
    }
}

void kprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
}

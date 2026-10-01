#include "shell.h"
#include "heap.h"
#include "io.h"
#include "keyboard.h"
#include "kprintf.h"
#include "kstring.h"
#include "multiboot.h"
#include "timer.h"
#include "vga.h"

extern "C" char kernel_start, kernel_end;

namespace {

constexpr int MAX_LINE = 128;
constexpr int MAX_ARGS = 8;

uint8_t text_fg = vga::LIGHT_GREY;

struct Command {
    const char* name;
    const char* help;
    void (*fn)(int argc, char** argv);
};

void cmd_help(int, char**);
void cmd_clear(int, char**);
void cmd_echo(int, char**);
void cmd_uptime(int, char**);
void cmd_sleep(int, char**);
void cmd_meminfo(int, char**);
void cmd_memmap(int, char**);
void cmd_heaptest(int, char**);
void cmd_color(int, char**);
void cmd_about(int, char**);
void cmd_crash(int, char**);
void cmd_reboot(int, char**);
void cmd_halt(int, char**);

const Command commands[] = {
    {"help",     "list commands",                    cmd_help},
    {"clear",    "clear the screen",                 cmd_clear},
    {"echo",     "print arguments",                  cmd_echo},
    {"uptime",   "time since boot",                  cmd_uptime},
    {"sleep",    "sleep <ms> using the PIT timer",   cmd_sleep},
    {"meminfo",  "RAM and kernel heap usage",        cmd_meminfo},
    {"memmap",   "bootloader memory map",            cmd_memmap},
    {"heaptest", "demo new/delete + constructors",   cmd_heaptest},
    {"color",    "color <0-15> sets the text color", cmd_color},
    {"about",    "about TinyOS",                     cmd_about},
    {"crash",    "trigger a divide-by-zero panic",   cmd_crash},
    {"reboot",   "restart the machine",              cmd_reboot},
    {"halt",     "stop the CPU",                     cmd_halt},
};
constexpr int command_count = sizeof(commands) / sizeof(commands[0]);

bool parse_uint(const char* s, uint32_t& out) {
    if (!*s) return false;
    uint32_t v = 0;
    for (; *s; s++) {
        if (*s < '0' || *s > '9') return false;
        v = v * 10 + (*s - '0');
    }
    out = v;
    return true;
}

void cmd_help(int, char**) {
    for (int i = 0; i < command_count; i++)
        kprintf("  %s%s%s\n", commands[i].name,
                strlen(commands[i].name) < 8 ? "\t\t" : "\t", commands[i].help);
}

void cmd_clear(int, char**) { vga::clear(); }

void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) kprintf("%s%s", argv[i], i + 1 < argc ? " " : "");
    kputc('\n');
}

void cmd_uptime(int, char**) {
    uint32_t t = timer::ticks();
    uint32_t s = t / timer::HZ;
    kprintf("up %u:%02u:%02u (%u ticks)\n", s / 3600, (s / 60) % 60, s % 60, t);
}

void cmd_sleep(int argc, char** argv) {
    uint32_t ms;
    if (argc < 2 || !parse_uint(argv[1], ms)) { kputs("usage: sleep <milliseconds>\n"); return; }
    timer::sleep_ms(ms);
    kprintf("slept %u ms\n", ms);
}

void cmd_meminfo(int, char**) {
    uint32_t total_kb = 1024 + bootmem::upper_kb();
    kprintf("RAM:          %u KiB (%u MiB)\n", total_kb, total_kb / 1024);
    kprintf("Kernel image: %p - %p (%u KiB)\n", &kernel_start, &kernel_end,
            (uint32_t)(&kernel_end - &kernel_start) / 1024);
    kprintf("Heap total:   %u KiB\n", (uint32_t)heap::total() / 1024);
    kprintf("Heap used:    %u bytes\n", (uint32_t)heap::used());
    kprintf("Heap free:    %u KiB\n", (uint32_t)heap::available() / 1024);
}

void cmd_memmap(int, char**) {
    if (!bootmem::region_count()) { kputs("no memory map from bootloader\n"); return; }
    for (int i = 0; i < bootmem::region_count(); i++) {
        const MemRegion& r = bootmem::region(i);
        kprintf("  %08x%08x  len %08x%08x  %s\n", r.base_high, r.base_low,
                r.len_high, r.len_low, r.type == 1 ? "usable" : "reserved");
    }
}

struct Probe {
    int id;
    explicit Probe(int i) : id(i) { kprintf("  Probe(%d) constructed at %p\n", id, this); }
    ~Probe() { kprintf("  Probe(%d) destroyed\n", id); }
};

void cmd_heaptest(int, char**) {
    kputs("C++ on bare metal:\n");
    Probe* a = new Probe(1);
    Probe* b = new Probe(2);
    int* numbers = new int[1000];
    for (int i = 0; i < 1000; i++) numbers[i] = i * i;
    kprintf("  numbers[999] = %d, heap used = %u bytes\n", numbers[999], (uint32_t)heap::used());
    delete a;
    delete b;
    delete[] numbers;
    kprintf("  after delete, heap used = %u bytes\n", (uint32_t)heap::used());
}

void cmd_color(int argc, char** argv) {
    uint32_t c;
    if (argc < 2 || !parse_uint(argv[1], c) || c > 15) { kputs("usage: color <0-15>\n"); return; }
    text_fg = c;
    vga::set_color(text_fg, vga::BLACK);
    kputs("text color changed\n");
}

void cmd_about(int, char**) {
    vga::set_color(vga::LIGHT_CYAN, vga::BLACK);
    kputs(R"ART(
  _____ _             ___  ____
 |_   _(_)_ __  _   _/ _ \/ ___|
   | | | | '_ \| | | | | \___ \
   | | | | | | | |_| | |_| |___) |
   |_| |_|_| |_|\__, |\___/|____/
                |___/
)ART");
    vga::set_color(text_fg, vga::BLACK);
    kputs("A tiny 32-bit x86 kernel written in C++.\n"
          "GDT, IDT, PIC, PIT, keyboard, heap, and this shell. Built from scratch.\n");
}

void cmd_crash(int, char**) {
    asm volatile("xor %%edx, %%edx\n\tmov $1, %%eax\n\txor %%ecx, %%ecx\n\tdiv %%ecx" ::: "eax", "ecx", "edx");
}

void cmd_reboot(int, char**) {
    while (inb(0x64) & 0x02) {}
    outb(0x64, 0xFE);                    // pulse the CPU reset line
    for (;;) asm volatile("hlt");
}

void cmd_halt(int, char**) {
    kputs("System halted. You can power off now.\n");
    asm volatile("cli");
    for (;;) asm volatile("hlt");
}

int read_line(char* line) {
    int len = 0;
    for (;;) {
        char c = keyboard::getchar();
        if (c == '\n') { kputc('\n'); break; }
        if (c == '\b') { if (len > 0) { len--; kputc('\b'); } continue; }
        if (c >= ' ' && c <= '~' && len < MAX_LINE - 1) { line[len++] = c; kputc(c); }
    }
    line[len] = '\0';
    return len;
}

int tokenize(char* line, char** argv) {
    int argc = 0;
    char* p = line;
    while (*p && argc < MAX_ARGS) {
        while (*p == ' ') *p++ = '\0';
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
    }
    return argc;
}

}  // namespace

namespace shell {

void run() {
    kputs("Type 'help' for a list of commands.\n\n");
    char line[MAX_LINE];
    char* argv[MAX_ARGS];

    for (;;) {
        vga::set_color(vga::LIGHT_GREEN, vga::BLACK);
        kputs("tinyos> ");
        vga::set_color(text_fg, vga::BLACK);

        if (!read_line(line)) continue;
        int argc = tokenize(line, argv);
        if (!argc) continue;

        bool found = false;
        for (int i = 0; i < command_count; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].fn(argc, argv);
                found = true;
                break;
            }
        }
        if (!found) kprintf("unknown command: %s (try 'help')\n", argv[0]);
    }
}

}  // namespace shell

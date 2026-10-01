ENTRY(_start)

SECTIONS
{
    . = 1M;
    kernel_start = .;

    .text : ALIGN(4K) {
        *(.multiboot)
        *(.text*)
    }
    .rodata : ALIGN(4K) {
        *(.rodata*)
    }
    .init_array : ALIGN(4) {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    }
    .data : ALIGN(4K) {
        *(.data*)
    }
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)
    }

    kernel_end = ALIGN(4K);

    /DISCARD/ : { *(.eh_frame) *(.comment) *(.note*) }
}

; boot.asm - Multiboot entry point. GRUB (or QEMU -kernel) loads us here.
MBALIGN  equ 1 << 0              ; align loaded modules on page boundaries
MEMINFO  equ 1 << 1              ; ask the bootloader for a memory map
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KiB kernel stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    xor ebp, ebp
    push ebx                     ; arg 2: multiboot info pointer
    push eax                     ; arg 1: multiboot magic (0x2BADB002)
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits

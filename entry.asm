MODULEALIGN equ 1 << 0
MEMINFO equ 1 << 1
VIDEO_MODE equ 1 << 2
FLAGS equ MODULEALIGN | MEMINFO | VIDEO_MODE
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)
section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 1024
    dd 768
    dd 16
section .text
global _start
    push ebx
    extern kmain
    call kmain
.halt:
    hlt
    jmp .halt
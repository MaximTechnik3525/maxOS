MAGIC equ 0x1BADB002
FLAGS equ 0x00000007
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
    dd 1280
    dd 720
    dd 16
section .text
global _start
    mov esp, stack_space
    push ebx
    extern kmain
    call kmain
.halt:
    hlt
    jmp .halt
section .bss
align 16
stack_bottom:
    resb 16384
stack_space:
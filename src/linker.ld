OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    . = 1M;
    _kernel_start = .;
    .boot : ALIGN(4)
    {
        KEEP(*(.multiboot_header))
    }
    .text : ALIGN(4096)
    {
        *(.text*)
    }
    .rodata : ALIGN(4096)
    {
        *(.rodata*)
    }
    .data : ALIGN(4096)
    {
        *(.data*)
    }
    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss*)
    }
    . = ALIGN(4096);
    _kernel_end = .;
}
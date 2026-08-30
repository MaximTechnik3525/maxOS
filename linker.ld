ENTRY(_start)
SECTIONS
{
    . = 1M;
    .boot : ALIGN(4)
    {
        KEEP(*(.multiboot_header))
    }
    .text : ALIGN(4)
    {
        *(.text)
    }
    .rodata : ALIGN(4)
    {
        *(.rodata)
    }
    .data : ALIGN(4)
    {
        *(.data)
    }
    .bss : ALIGN(16)
    {
        *(COMMON)
        *(.bss)
    }
}
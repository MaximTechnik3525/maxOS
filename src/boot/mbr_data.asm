section .rodata
global mbr_boot_code
mbr_boot_code:
    incbin "boot_mbr.bin"

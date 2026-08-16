[BITS 16]
[org 0x7c00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov ah, 0x02
    mov al, 40
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, 0x1000
    int 0x13
    jc disk_err
    xor ax, ax
    mov es, ax

    mov ax, 0x4F01
    mov cx, 0x114
    mov di, vesa_info_block
    int 0x10
    jc disk_err

    mov ax, 0x4F02
    mov bx, 0x4114
    int 0x10
    jc disk_err

    mov si, vesa_info_block
    mov di, 0x9000
    mov cx, 64
copy_vesa:
    mov eax, [si]
    mov [di], eax
    add si, 4
    add di, 4
    loop copy_vesa

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_pm

[BITS 32]

init_pm:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x1000

disk_err:
    jmp $

gdt_start:
    dd 0x0, 0x0

gdt_code:
    dw 0xFFFF, 0x0, 0x9A00, 0x00CF

gdt_data:
    dw 0xFFFF, 0x0, 0x9200, 0x00CF

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
vesa_info_block: times 256 db 0
times 510-($-$$) db 0
dw 0xaa55
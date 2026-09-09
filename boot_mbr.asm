[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; 1. Query VBE Mode 0x117 (1024x768x16) Info
    mov ax, 0x4F01
    mov cx, 0x117
    mov di, 0x8000          ; Buffer at 0x8000
    int 0x10
    cmp ax, 0x004F
    jne boot_fail

    ; Extract PhysBasePtr (offset 40 in ModeInfoBlock)
    mov eax, [0x8000 + 40]
    mov [fb_addr], eax

    ; 2. Set VBE Mode 0x4117 (1024x768x16 Linear Framebuffer)
    mov ax, 0x4F02
    mov bx, 0x4117
    int 0x10
    cmp ax, 0x004F
    jne boot_fail

    ; 3. Read Kernel (120 sectors = 60 KB) from LBA 32 to 0x1000:0000 (physical 0x10000)
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc boot_fail

    ; Read second chunk (120 sectors = 60 KB) from LBA 152 to 0x1F00:0000 (physical 0x1F000)
    mov word [dap + 2], 120
    mov word [dap + 6], 0x1F00
    mov dword [dap + 8], 152
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc boot_fail

    ; 4. Fast A20 gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Mask all PIC interrupts
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    ; 5. Enter 32-bit Protected Mode
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:pm_start

boot_fail:
    cli
    hlt
    jmp boot_fail

[BITS 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Check if loaded image has ELF magic (0x7F, 'E', 'L', 'F')
    cmp dword [0x10000], 0x464C457F
    je .load_elf

    ; If raw binary, copy directly to 0x100000 and jump to 0x101000
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, (240 * 512) / 4
    rep movsd
    ; Clear BSS (from 0x100000 + 240*512 to 0x150000)
    mov edi, 0x100000 + (240 * 512)
    mov ecx, (64 * 1024) / 4
    xor eax, eax
    rep stosd
    mov dword [k_entry], 0x101000
    jmp .enter_kernel

.load_elf:
    mov edx, [0x10000 + 24]         ; e_entry (lower 32-bits)
    mov [k_entry], edx
    mov esi, 0x10000
    add esi, [0x10000 + 32]         ; e_phoff (ELF64 offset 32)
    movzx ecx, word [0x10000 + 56]  ; e_phnum (ELF64 offset 56)

.ph_loop:
    cmp dword [esi], 1              ; PT_LOAD
    jne .ph_next
    push esi
    push ecx
    mov ebx, [esi + 8]              ; p_offset (ELF64 offset 8)
    add ebx, 0x10000
    mov edi, [esi + 24]             ; p_paddr (ELF64 offset 24)
    mov ecx, [esi + 32]             ; p_filesz (ELF64 offset 32)
    mov esi, ebx
    rep movsb
    pop ecx
    pop esi
    mov eax, [esi + 40]             ; p_memsz (ELF64 offset 40)
    sub eax, [esi + 32]
    jbe .ph_next
    push ecx
    mov ecx, eax
    xor al, al
    rep stosb
    pop ecx

.ph_next:
    add esi, 56                     ; ELF64 program header entry size = 56 bytes
    loop .ph_loop

.enter_kernel:
    ; Setup Multiboot structure at 0x9000
    mov edi, 0x9000
    mov ecx, 32
    xor eax, eax
    rep stosd

    ; Multiboot flags: bit 11 (framebuffer info valid)
    mov dword [0x9000], 0x00000800
    mov eax, [fb_addr]
    mov [0x9000 + 88], eax          ; framebuffer_addr (low 32)
    mov dword [0x9000 + 92], 0      ; framebuffer_addr (high 32)
    mov dword [0x9000 + 96], 2048   ; pitch
    mov dword [0x9000 + 100], 1024  ; width
    mov dword [0x9000 + 104], 768   ; height
    mov byte [0x9000 + 108], 16     ; bpp
    mov byte [0x9000 + 109], 1      ; type RGB

    ; Jump to kernel with Multiboot parameters
    mov eax, 0x2BADB002
    mov ebx, 0x9000
    mov edx, [k_entry]
    jmp edx

align 4
dap:
    db 16           ; packet size
    db 0            ; reserved
    dw 120          ; sector count (120 sectors = 60 KB)
    dw 0x0000       ; offset
    dw 0x1000       ; segment 0x1000 (physical 0x10000)
    dq 32           ; LBA 32

boot_drive: db 0
fb_addr:    dd 0
k_entry:    dd 0

align 4
gdt_start:
    dq 0
    ; Code: base=0, limit=0xFFFFF, G=1, D=1, P=1, DPL=0, Type=0xA (exec/read)
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00
    ; Data: base=0, limit=0xFFFFF, G=1, B=1, P=1, DPL=0, Type=0x2 (read/write)
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55

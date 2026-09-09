MODULEALIGN equ 1 << 0
MEMINFO     equ 1 << 1
VIDEO_MODE  equ 1 << 2
FLAGS       equ MODULEALIGN | MEMINFO | VIDEO_MODE
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

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
[bits 32]
global _start
_start:
    cli

    ; Save Multiboot parameters (eax = magic, ebx = multiboot_info address)
    mov [saved_mb_magic], eax
    mov [saved_mb_info], ebx

    ; Set initial 32-bit stack
    mov esp, stack_top

    ; --------------------------------------------------------------------------
    ; 1. Build 4-Level Page Tables (PML4 -> PDPT -> PD -> 2MB Huge Pages)
    ;    Identity-map the entire first 4 GB of physical memory.
    ;    This covers kernel, RAM, MMIO, and VESA VBE linear framebuffer.
    ; --------------------------------------------------------------------------

    ; Clear PML4, PDPT, and the 4 PD tables (6 * 4096 = 24576 bytes)
    mov edi, pml4_table
    mov ecx, (6 * 4096) / 4
    xor eax, eax
    rep stosd

    ; Point PML4[0] to pdpt_table (Present | Writable | User = 0x07)
    mov eax, pdpt_table
    or eax, 0x07
    mov dword [pml4_table], eax

    ; Point PDPT[0..3] to pd_table_0 .. pd_table_3 (covers 0..4 GB, User accessible)
    mov eax, pd_table_0
    or eax, 0x07
    mov dword [pdpt_table + 0], eax

    mov eax, pd_table_1
    or eax, 0x07
    mov dword [pdpt_table + 8], eax

    mov eax, pd_table_2
    or eax, 0x07
    mov dword [pdpt_table + 16], eax

    mov eax, pd_table_3
    or eax, 0x07
    mov dword [pdpt_table + 24], eax

    ; Map 2048 2MB huge pages across the 4 PD tables:
    ; page_addr = ecx * 2MB
    ; flags = 0x87 (Present | Writable | User | Huge 2MB)
    mov ecx, 0
.map_pages:
    mov eax, ecx
    shl eax, 21             ; eax = ecx * 2097152 (2MB)
    or eax, 0x87            ; Present | Writable | User | Huge 2MB
    mov dword [pd_table_0 + ecx * 8], eax
    mov dword [pd_table_0 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 2048           ; 2048 * 2MB = 4096 MB = 4 GB
    jne .map_pages

    ; --------------------------------------------------------------------------
    ; 2. Enable PAE, Long Mode, and Paging
    ; --------------------------------------------------------------------------

    ; Load CR3 with physical address of PML4
    mov eax, pml4_table
    mov cr3, eax

    ; Enable PAE (bit 5), OSFXSR (bit 9), OSXMMEXCPT (bit 10) in CR4
    mov eax, cr4
    or eax, (1 << 5) | (1 << 9) | (1 << 10)
    mov cr4, eax

    ; Set LME (Long Mode Enable, bit 8) and SCE (System Call Enable, bit 0) in EFER MSR (0xC0000080)
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8) | (1 << 0)
    wrmsr

    ; Enable Paging (bit 31), Protection (bit 0), MP (bit 1), clear EM (bit 2) in CR0
    mov eax, cr0
    and eax, ~(1 << 2)          ; Clear EM
    or eax, (1 << 31) | (1 << 0) | (1 << 1) ; PG | PE | MP
    mov cr0, eax

    ; --------------------------------------------------------------------------
    ; 3. Load 64-bit GDT and Far Jump to Long Mode
    ; --------------------------------------------------------------------------
    lgdt [gdt64_desc]
    jmp 0x08:.realm64

; ------------------------------------------------------------------------------
; 64-Bit Long Mode Execution
; ------------------------------------------------------------------------------
[bits 64]
.realm64:
    ; Load 64-bit data segment selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Initialize 64-bit stack pointer
    mov rsp, stack_top

    ; Pass Multiboot arguments according to System V AMD64 ABI:
    ; 1st argument = RDI (multiboot_info_address)
    ; 2nd argument = RSI (magic)
    mov edi, [saved_mb_info]
    mov esi, [saved_mb_magic]

    ; Call 64-bit C kernel
    extern kmain
    call kmain

.halt:
    cli
    hlt
    jmp .halt

; ------------------------------------------------------------------------------
; GDT Table for 64-bit Long Mode with Ring 3 and TSS Descriptors
; ------------------------------------------------------------------------------
section .data
align 16
global gdt64
global gdt64_desc
global gdt64_tss_entry
gdt64:
    dq 0x0000000000000000       ; 0x00: Null descriptor
    dq 0x00209A0000000000       ; 0x08: 64-bit Kernel Code (DPL=0, L=1, Exec/Read)
    dq 0x0000920000000000       ; 0x10: 64-bit Kernel Data (DPL=0, Read/Write)
    dq 0x0000F20000000000       ; 0x18: 32-bit User Data / Compat (STAR user base)
    dq 0x0000F20000000000       ; 0x20: 64-bit User Data (DPL=3, Read/Write)
    dq 0x0020FA0000000000       ; 0x28: 64-bit User Code (DPL=3, L=1, Exec/Read)
gdt64_tss_entry:
    dq 0x0000000000000000       ; 0x30: 64-bit TSS Low Qword (filled at runtime)
    dq 0x0000000000000000       ; 0x38: 64-bit TSS High Qword (filled at runtime)
gdt64_end:

gdt64_desc:
    dw gdt64_end - gdt64 - 1    ; Limit (16 bits)
    dd gdt64                    ; Base address (32 bits)

; ------------------------------------------------------------------------------
; Page Tables, Stack, and Parameters
; ------------------------------------------------------------------------------
section .bss
align 4096
global pml4_table
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table_0:
    resb 4096
pd_table_1:
    resb 4096
pd_table_2:
    resb 4096
pd_table_3:
    resb 4096

align 16
global stack_bottom
global stack_top
stack_bottom:
    resb 65536                  ; 64 KB kernel stack
stack_top:

saved_mb_info:
    resd 1
saved_mb_magic:
    resd 1
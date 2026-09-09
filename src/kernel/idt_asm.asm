[bits 64]
section .text

global load_idt
global irq0_timer_entry
global gp_fault_entry
global page_fault_entry
global default_exception_entry
global interrupt_stack_top

extern timer_irq_handler
extern schedule_tick
extern gp_fault_handler
extern page_fault_handler

section .bss
align 16
interrupt_stack_bottom:
    resb 16384
interrupt_stack_top:

section .text

; ------------------------------------------------------------------------------
; load_idt:
; Loads IDTR with pointer passed in RDI.
; ------------------------------------------------------------------------------
load_idt:
    lidt [rdi]
    ret

; ------------------------------------------------------------------------------
; irq0_timer_entry:
; Hardware interrupt vector 32 (IRQ 0 - PIT Timer).
; Hardware automatically pushes: SS, RSP, RFLAGS, CS, RIP
; ------------------------------------------------------------------------------
irq0_timer_entry:
    ; Push all 15 general-purpose registers (Complete CPU context)
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; 15 quadwords (120 bytes) + 5 hardware quadwords (40 bytes) = 160 bytes (16-byte aligned)
    mov rdi, rsp                ; 1st param: current saved RSP
    call schedule_tick          ; Schedule next task; RAX = new task RSP
    mov rsp, rax                ; Switch stack to chosen task!

    ; Send End of Interrupt (EOI = 0x20) to Master PIC (0x20)
    mov al, 0x20
    out 0x20, al

    ; Check if target task is Ring 3 (CS RPL == 3)
    ; In trap_frame, CS is at [rsp + 15*8 + 8] = [rsp + 128]
    test byte [rsp + 128], 3
    jz .to_ring0
    mov bx, 0x23                ; User Data Segment (0x20 | 3)
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    jmp .pop_all
.to_ring0:
    mov bx, 0x10                ; Kernel Data Segment (0x10)
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
.pop_all:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq

; ------------------------------------------------------------------------------
; gp_fault_entry (Vector 13 - General Protection Fault #GP):
; Hardware pushed: SS, RSP, RFLAGS, CS, RIP, ERROR CODE
; ------------------------------------------------------------------------------
gp_fault_entry:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    mov rdi, [rsp + 72]         ; 1st arg: error code
    mov rsi, [rsp + 80]         ; 2nd arg: faulting RIP
    mov rdx, [rsp + 88]         ; 3rd arg: faulting CS
    sub rsp, 8
    call gp_fault_handler
    add rsp, 8

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    add rsp, 8                  ; Pop error code from stack!
    iretq

; ------------------------------------------------------------------------------
; page_fault_entry (Vector 14 - Page Fault #PF):
; Hardware pushed: SS, RSP, RFLAGS, CS, RIP, ERROR CODE
; ------------------------------------------------------------------------------
page_fault_entry:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    mov rdi, [rsp + 72]         ; 1st arg: error code
    mov rsi, [rsp + 80]         ; 2nd arg: faulting RIP
    mov rdx, [rsp + 88]         ; 3rd arg: faulting CS
    sub rsp, 8
    call page_fault_handler
    add rsp, 8

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    add rsp, 8                  ; Pop error code from stack!
    iretq

; ------------------------------------------------------------------------------
; default_exception_entry:
; Default trap/interrupt handler for vectors without error codes.
; ------------------------------------------------------------------------------
default_exception_entry:
    push rax
    mov al, 0x20
    out 0x20, al
    pop rax
    iretq

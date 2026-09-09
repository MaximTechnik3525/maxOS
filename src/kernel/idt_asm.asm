[bits 64]
section .text

global load_idt
global irq0_timer_entry
global default_exception_entry
global interrupt_stack_top

extern timer_irq_handler

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
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    ; 16-byte stack alignment check
    sub rsp, 8
    call timer_irq_handler
    add rsp, 8

    ; Send End of Interrupt (EOI = 0x20) to Master PIC (0x20)
    mov al, 0x20
    out 0x20, al

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    iretq

; ------------------------------------------------------------------------------
; default_exception_entry:
; Default trap/interrupt handler.
; ------------------------------------------------------------------------------
default_exception_entry:
    push rax
    mov al, 0x20
    out 0x20, al
    pop rax
    iretq

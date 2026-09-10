[bits 64]
section .text

global load_idt
global irq0_timer_entry
global isr_stub_table
global irq_stub_table
global isr_unhandled
global interrupt_stack_top

; Legacy symbols
global divide_error_entry
global invalid_opcode_entry
global double_fault_entry
global gp_fault_entry
global page_fault_entry
global default_exception_entry

extern schedule_tick
extern exception_dispatcher
extern irq_default_handler

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
    ; Push all 15 general-purpose registers (matching task_t trap_frame)
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
    mov rdi, rsp                ; 1st param: current saved RSP (trap_frame*)
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
; Unified Exception Handling (Vectors 0..31)
; ------------------------------------------------------------------------------
%macro ISR_NOERR 1
isr_stub_%1:
    push qword 0                ; Dummy error code
    push qword %1               ; Vector number
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push qword %1               ; Vector number (error code already pushed by CPU)
    jmp isr_common_stub
%endmacro

; Exception vectors 0..31
ISR_NOERR 0   ; #DE Divide Error
ISR_NOERR 1   ; #DB Debug
ISR_NOERR 2   ; NMI
ISR_NOERR 3   ; #BP Breakpoint
ISR_NOERR 4   ; #OF Overflow
ISR_NOERR 5   ; #BR BOUND Range Exceeded
ISR_NOERR 6   ; #UD Invalid Opcode
ISR_NOERR 7   ; #NM Device Not Available
ISR_ERR   8   ; #DF Double Fault (has error code)
ISR_NOERR 9   ; Coprocessor Segment Overrun
ISR_ERR   10  ; #TS Invalid TSS (has error code)
ISR_ERR   11  ; #NP Segment Not Present (has error code)
ISR_ERR   12  ; #SS Stack-Segment Fault (has error code)
ISR_ERR   13  ; #GP General Protection (has error code)
ISR_ERR   14  ; #PF Page Fault (has error code)
ISR_NOERR 15  ; Reserved
ISR_NOERR 16  ; #MF x87 FPU Error
ISR_ERR   17  ; #AC Alignment Check (has error code)
ISR_NOERR 18  ; #MC Machine Check
ISR_NOERR 19  ; #XF SIMD Floating-Point
ISR_NOERR 20  ; #VE Virtualization Exception
ISR_ERR   21  ; #CP Control Protection (has error code)
ISR_NOERR 22  ; Reserved
ISR_NOERR 23  ; Reserved
ISR_NOERR 24  ; Reserved
ISR_NOERR 25  ; Reserved
ISR_NOERR 26  ; Reserved
ISR_NOERR 27  ; Reserved
ISR_NOERR 28  ; Hypervisor Injection
ISR_ERR   29  ; #SX Security Exception (has error code)
ISR_ERR   30  ; #VC VMM Communication (has error code)
ISR_NOERR 31  ; Reserved

isr_common_stub:
    ; Push 15 general-purpose registers (r15 down to rax)
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

    mov rdi, rsp                ; 1st parameter: pointer to isr_frame_t
    call exception_dispatcher

    ; Restore registers
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

    add rsp, 16                 ; Pop vector number and error code
    iretq

; ------------------------------------------------------------------------------
; Hardware IRQs 1..15 (Vectors 33..47)
; ------------------------------------------------------------------------------
%macro IRQ_STUB 2
irq_stub_%1:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    mov rdi, %2                 ; IRQ number
    call irq_default_handler

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
%endmacro

IRQ_STUB 1, 1
IRQ_STUB 2, 2
IRQ_STUB 3, 3
IRQ_STUB 4, 4
IRQ_STUB 5, 5
IRQ_STUB 6, 6
IRQ_STUB 7, 7
IRQ_STUB 8, 8
IRQ_STUB 9, 9
IRQ_STUB 10, 10
IRQ_STUB 11, 11
IRQ_STUB 12, 12
IRQ_STUB 13, 13
IRQ_STUB 14, 14
IRQ_STUB 15, 15

; ------------------------------------------------------------------------------
; Unhandled Interrupt Stub (Vectors 48..255)
; ------------------------------------------------------------------------------
isr_unhandled:
    iretq

; ------------------------------------------------------------------------------
; Legacy aliases
; ------------------------------------------------------------------------------
divide_error_entry:      jmp isr_stub_0
invalid_opcode_entry:    jmp isr_stub_6
double_fault_entry:      jmp isr_stub_8
gp_fault_entry:          jmp isr_stub_13
page_fault_entry:        jmp isr_stub_14
default_exception_entry: jmp isr_unhandled

; ------------------------------------------------------------------------------
; Pointer Tables
; ------------------------------------------------------------------------------
section .rodata
align 8
isr_stub_table:
    dq isr_stub_0,  isr_stub_1,  isr_stub_2,  isr_stub_3
    dq isr_stub_4,  isr_stub_5,  isr_stub_6,  isr_stub_7
    dq isr_stub_8,  isr_stub_9,  isr_stub_10, isr_stub_11
    dq isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15
    dq isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19
    dq isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23
    dq isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27
    dq isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31

irq_stub_table:
    dq irq0_timer_entry
    dq irq_stub_1,  irq_stub_2,  irq_stub_3,  irq_stub_4
    dq irq_stub_5,  irq_stub_6,  irq_stub_7,  irq_stub_8
    dq irq_stub_9,  irq_stub_10, irq_stub_11, irq_stub_12
    dq irq_stub_13, irq_stub_14, irq_stub_15

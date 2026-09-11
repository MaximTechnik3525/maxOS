[bits 64]
section .text

global syscall_entry_asm
global enter_ring3
global run_in_ring3
global exit_to_kernel
global load_tss
global get_cpl
global read_cs
global read_ss

extern syscall_dispatcher
extern kernel_stack_top

section .bss
align 16
scratch_rsp:          resq 1
saved_kernel_rsp:     resq 1

section .text

; ------------------------------------------------------------------------------
; enter_ring3:
; Drops CPU privilege level from Ring 0 to Ring 3 via iretq.
; Arguments (System V AMD64 ABI):
;   RDI = User entry point (RIP)
;   RSI = User stack pointer (RSP)
; ------------------------------------------------------------------------------
enter_ring3:
    ; Set User Data selector (0x20 | 3 = 0x23) into segment registers
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Construct 64-bit iretq stack frame:
    ; [SS]
    ; [RSP]
    ; [RFLAGS]
    ; [CS]
    ; [RIP]
    push 0x23                   ; User SS (Selector 0x20 | RPL 3)
    push rsi                    ; User RSP
    pushfq
    pop rax
    or rax, 0x200               ; Enable IF=1 (IDT is active, timer ticks)
    push rax                    ; RFLAGS
    push 0x2B                   ; User CS (Selector 0x28 | RPL 3)
    push rdi                    ; User RIP
    iretq                       ; Hardware privilege transition to Ring 3!

; ------------------------------------------------------------------------------
; run_in_ring3:
; Drops to Ring 3 while preserving the calling kernel context.
; Returns cleanly when SYS_EXIT triggers exit_to_kernel.
; Arguments:
;   RDI = User entry point (RIP)
;   RSI = User stack pointer (RSP)
; ------------------------------------------------------------------------------
run_in_ring3:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    mov [saved_kernel_rsp], rsp

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23                   ; User SS
    push rsi                    ; User RSP
    pushfq
    pop rax
    or rax, 0x200               ; Enable IF=1 (IDT is active, timer ticks)
    push rax                    ; RFLAGS
    push 0x2B                   ; User CS
    push rdi                    ; User RIP
    iretq

; ------------------------------------------------------------------------------
; exit_to_kernel:
; Restores kernel context saved by run_in_ring3 and returns to caller.
; ------------------------------------------------------------------------------
exit_to_kernel:
    mov rsp, [saved_kernel_rsp]
    mov ax, 0x10                ; Restore Kernel DS
    mov ds, ax
    mov es, ax
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret

; ------------------------------------------------------------------------------
; syscall_entry_asm:
; Entered on SYSCALL instruction executed in Ring 3.
; Hardware automatically:
;   RCX <- User RIP
;   R11 <- User RFLAGS
;   CS  <- STAR[47:32] (0x08, Ring 0)
;   SS  <- STAR[47:32] + 8 (0x10, Ring 0)
;   RIP <- LSTAR (this function)
; ------------------------------------------------------------------------------
extern current_kstack_top

syscall_entry_asm:
    ; 1. Switch from untrusted User RSP to trusted dedicated Kernel Syscall RSP
    mov [scratch_rsp], rsp
    mov rsp, [current_kstack_top]

    ; 2. Preserve ALL user registers on kernel stack (15 quadwords = 120 bytes)
    push qword [scratch_rsp] ; Save original user RSP
    push r11                    ; Save user RFLAGS
    push rcx                    ; Save user RIP
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    push rdi                    ; Save user arg 1 / scratch
    push rsi                    ; Save user arg 2 / scratch
    push rdx                    ; Save user arg 3 / scratch
    push r10                    ; Save user arg 4 / scratch
    push r8                     ; Save user arg 5 / scratch
    push r9                     ; Save user arg 6 / scratch

    ; 16-byte stack alignment check before call:
    ; 15 pushes (120 bytes) + 8 = 128 bytes (128 is multiple of 16)
    sub rsp, 8

    ; 3. Map user syscall arguments to System V C calling convention:
    ; In user space:
    ;   RAX = syscall number
    ;   RDI = arg 1
    ;   RSI = arg 2
    ;   RDX = arg 3
    ;   R10 = arg 4
    ;   R8  = arg 5
    ;
    ; In System V C ABI for syscall_dispatcher(num, a1, a2, a3, a4, a5):
    ;   1st param: RDI (num)
    ;   2nd param: RSI (a1)
    ;   3rd param: RDX (a2)
    ;   4th param: RCX (a3)
    ;   5th param: R8  (a4)
    ;   6th param: R9  (a5)
    mov rcx, rdx                ; arg 3 -> 4th param (RCX)
    mov rdx, rsi                ; arg 2 -> 3rd param (RDX)
    mov rsi, rdi                ; arg 1 -> 2nd param (RSI)
    mov rdi, rax                ; num   -> 1st param (RDI)
    mov r9, r8                  ; arg 5 -> 6th param (R9)
    mov r8, r10                 ; arg 4 -> 5th param (R8)
    sti                         ; Enable interrupts during syscall execution
    call syscall_dispatcher
    cli                         ; Disable interrupts for atomic user context restoration
    ; Return value from C dispatcher is in RAX!

    add rsp, 8

    ; 4. Restore preserved user registers from kernel stack
    pop r9
    pop r8
    pop r10
    pop rdx
    pop rsi
    pop rdi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop rcx                     ; Restore user RIP
    pop r11                     ; Restore user RFLAGS
    pop qword [scratch_rsp]

    ; 5. Restore user stack pointer and return to Ring 3
    mov rsp, [scratch_rsp]

    ; sysretq: restores RIP from RCX, RFLAGS from R11, CS=0x2B, SS=0x23
    o64 sysret

; ------------------------------------------------------------------------------
; load_tss:
; Loads Task Register with specified GDT selector.
; Argument: DI = TSS Selector (0x30)
; ------------------------------------------------------------------------------
load_tss:
    ltr di
    ret

; ------------------------------------------------------------------------------
; get_cpl:
; Returns Current Privilege Level (CS & 3).
; ------------------------------------------------------------------------------
get_cpl:
    mov ax, cs
    and rax, 3
    ret

read_cs:
    xor rax, rax
    mov ax, cs
    ret

read_ss:
    xor rax, rax
    mov ax, ss
    ret

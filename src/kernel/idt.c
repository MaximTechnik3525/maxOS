#include "idt.h"
#include "kernel.h"
#include "debug.h"
#include "user.h"
#include "maxp.h"
#include "task.h"
#include "string.h"

// 256 IDT Descriptors in 64-bit Long Mode
static struct idt_entry_64 idt64[256] __attribute__((aligned(16)));
static struct idt_ptr_64 idtr;

volatile unsigned long long system_ticks = 0;

extern void isr_unhandled(void);

static inline void io_wait(void) {
    outb(0x80, 0);
}

void idt_set_gate(int num, void* handler, unsigned char type_attr) {
    unsigned long long addr = (unsigned long long)handler;
    idt64[num].offset_low  = (unsigned short)(addr & 0xFFFF);
    idt64[num].selector    = 0x08; // Kernel 64-bit Code Segment
    idt64[num].ist         = 0;
    idt64[num].type_attr   = type_attr;
    idt64[num].offset_mid  = (unsigned short)((addr >> 16) & 0xFFFF);
    idt64[num].offset_high = (unsigned int)((addr >> 32) & 0xFFFFFFFF);
    idt64[num].zero        = 0;
}

void pic_remap(void) {
    // ICW1: Start initialization sequence
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();

    // ICW2: Vector offsets: Master = 0x20 (32), Slave = 0x28 (40)
    outb(0x21, 0x20);
    io_wait();
    outb(0xA1, 0x28);
    io_wait();

    // ICW3: Master PIC at IRQ2, Slave at cascade identity 2
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();

    // ICW4: 8086/88 mode
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    // Masks: Enable only IRQ 0 (Timer) on Master PIC, mask all others for safe polling
    outb(0x21, 0xFE); // 1111 1110: IRQ 0 enabled
    outb(0xA1, 0xFF); // all slave IRQs masked
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20); // Slave PIC EOI
    }
    outb(0x20, 0x20);     // Master PIC EOI
}

void pic_set_mask(unsigned char irq) {
    unsigned short port;
    unsigned char value;
    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) | (unsigned char)(1 << irq);
    outb(port, value);
}

void pic_clear_mask(unsigned char irq) {
    unsigned short port;
    unsigned char value;
    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) & (unsigned char)(~(1 << irq));
    outb(port, value);
}

void pit_init(unsigned int freq_hz) {
    if (freq_hz == 0) freq_hz = 1000;
    unsigned int divisor = 1193182 / freq_hz;
    // Channel 0, lobyte/hibyte, rate generator mode 2
    outb(0x43, 0x34);
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

unsigned long long get_uptime_ms(void) {
    return system_ticks;
}

static const char* exception_names[32] = {
    "Divide-by-zero (#DE)",
    "Debug (#DB)",
    "Non-maskable Interrupt (#NMI)",
    "Breakpoint (#BP)",
    "Overflow (#OF)",
    "Bound Range Exceeded (#BR)",
    "Invalid Opcode (#UD)",
    "Device Not Available (#NM)",
    "Double Fault (#DF)",
    "Coprocessor Segment Overrun",
    "Invalid TSS (#TS)",
    "Segment Not Present (#NP)",
    "Stack-Segment Fault (#SS)",
    "General Protection Fault (#GP)",
    "Page Fault (#PF)",
    "Reserved",
    "x87 FPU Error (#MF)",
    "Alignment Check (#AC)",
    "Machine Check (#MC)",
    "SIMD Floating-Point (#XF)",
    "Virtualization Exception (#VE)",
    "Control Protection (#CP)",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection",
    "VMM Communication (#VC)",
    "Security Exception (#SX)",
    "Reserved"
};

void exception_dispatcher(isr_frame_t* frame) {
    const char* name = (frame->vector < 32) ? exception_names[frame->vector] : "Unknown Exception";

    debug_puts("\n================ [CPU EXCEPTION] ================\n");
    debug_puts(" Exception:  ");
    debug_puts(name);
    debug_puts("\n Vector:     0x");
    debug_print_num(frame->vector, 16);
    debug_puts("   Error Code: 0x");
    debug_print_num(frame->error_code, 16);
    debug_puts("\n RIP:        0x");
    debug_print_num(frame->rip, 16);
    debug_puts("   CS:         0x");
    debug_print_num(frame->cs, 16);
    debug_puts("\n RSP:        0x");
    debug_print_num(frame->rsp, 16);
    debug_puts("   SS:         0x");
    debug_print_num(frame->ss, 16);
    debug_puts("\n RFLAGS:     0x");
    debug_print_num(frame->rflags, 16);
    debug_puts("\n");

    if (frame->vector == 14) { // Page Fault
        unsigned long long cr2;
        __asm__ __volatile__("mov %%cr2, %0" : "=r"(cr2));
        debug_puts(" CR2 (Faulting Memory Addr): 0x");
        debug_print_num(cr2, 16);
        debug_puts("\n");
    }

    debug_puts(" Registers:  RAX=0x");
    debug_print_num(frame->rax, 16);
    debug_puts(" RBX=0x");
    debug_print_num(frame->rbx, 16);
    debug_puts(" RCX=0x");
    debug_print_num(frame->rcx, 16);
    debug_puts(" RDX=0x");
    debug_print_num(frame->rdx, 16);
    debug_puts("\n=================================================\n");

    // Ring 3 user mode application crash -> isolate and terminate cleanly
    if ((frame->cs & 3) == 3) {
        debug_log("CPU", "Fault in Ring 3 User Mode. Terminating user application.");
        maxp_close_all_windows();
        draw_window();
        if (ring3_is_app_active()) {
            exit_to_kernel();
        } else {
            task_exit();
        }
        return;
    }

    // Ring 0 kernel crash -> Blue Screen / Kernel Panic
    debug_log("PANIC", "Fatal Kernel Exception in Ring 0! Halting CPU.");
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x001F); // Blue screen
    print_string("maxOS KERNEL PANIC - CRITICAL EXCEPTION", 280, 200, 0xFFFF);
    print_string((char*)name, 280, 230, 0xFFE0);
    print_string("System halted to protect hardware integrity and data.", 280, 260, 0xFFFF);

    while (1) {
        __asm__ __volatile__("cli; hlt");
    }
}

void irq_default_handler(unsigned long long irq) {
    // Spurious IRQ detection
    if (irq == 7) {
        outb(0x20, 0x0B); // Read ISR from Master
        unsigned char isr = inb(0x20);
        if (!(isr & 0x80)) {
            return; // Spurious IRQ 7, do not send EOI
        }
    } else if (irq == 15) {
        outb(0xA0, 0x0B); // Read ISR from Slave
        unsigned char isr = inb(0xA0);
        if (!(isr & 0x80)) {
            outb(0x20, 0x20); // Spurious IRQ 15, send EOI to Master only
            return;
        }
    }
    pic_send_eoi((unsigned char)irq);
}

void idt_init(void) {
    // 1. Install all 32 CPU exception handlers (vectors 0..31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, isr_stub_table[i], 0x8E); // Ring 0 Interrupt Gate
    }

    // 2. Install hardware IRQs 0..15 (vectors 32..47)
    for (int i = 0; i < 16; i++) {
        idt_set_gate(32 + i, irq_stub_table[i], 0x8E);
    }

    // 3. Fill remaining gates (48..255) with unhandled stub
    for (int i = 48; i < 256; i++) {
        idt_set_gate(i, isr_unhandled, 0x8E);
    }

    // 4. Load IDTR
    idtr.limit = sizeof(idt64) - 1;
    idtr.base = (unsigned long long)&idt64;
    load_idt(&idtr);

    // 5. Remap PIC and configure PIT to 1000 Hz (1ms per tick)
    pic_remap();
    pit_init(1000);

    // 6. Enable hardware interrupts
    __asm__ __volatile__("sti");
}

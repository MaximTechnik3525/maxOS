#include "idt.h"
#include "kernel.h"
#include "debug.h"
#include "user.h"
#include "maxp.h"
#include "task.h"

// 256 IDT Descriptors in 64-bit Long Mode
static struct idt_entry_64 idt64[256] __attribute__((aligned(16)));
static struct idt_ptr_64 idtr;

volatile unsigned long long system_ticks = 0;

static inline void io_wait(void) {
    outb(0x80, 0);
}

static void set_idt_gate(int num, void* handler, unsigned char type_attr) {
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

    // ICW3: Tell Master PIC about Slave at IRQ2, and Slave its cascade identity
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();

    // ICW4: 8086/88 mode
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    // Masks: Unmask IRQ 0 (Timer) on Master PIC, mask all others for safe polling
    outb(0x21, 0xFE); // 1111 1110: only IRQ 0 enabled
    outb(0xA1, 0xFF); // all slave IRQs masked
}

void pit_init(unsigned int freq_hz) {
    if (freq_hz == 0) freq_hz = 1000;
    unsigned int divisor = 1193182 / freq_hz;
    // Channel 0, lobyte/hibyte, rate generator mode 2
    outb(0x43, 0x34);
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void timer_irq_handler(void) {
    system_ticks++;
}

unsigned long long get_uptime_ms(void) {
    return system_ticks;
}

void divide_error_handler(unsigned long long rip, unsigned long long cs) {
    debug_puts("[CPU] #DE Divide by Zero! RIP=0x");
    debug_print_num(rip, 16);
    debug_puts(" CS=0x");
    debug_print_num(cs, 16);
    debug_puts("\n");

    if ((cs & 3) == 3) {
        debug_log("CPU", "Divide by zero in Ring 3 User Mode. Terminating application.");
        maxp_close_all_windows();
        draw_window();
        if (ring3_is_app_active()) {
            exit_to_kernel();
        } else {
            task_exit();
        }
    }
}

void invalid_opcode_handler(unsigned long long rip, unsigned long long cs) {
    debug_puts("[CPU] #UD Invalid Opcode! RIP=0x");
    debug_print_num(rip, 16);
    debug_puts(" CS=0x");
    debug_print_num(cs, 16);
    debug_puts("\n");

    if ((cs & 3) == 3) {
        debug_log("CPU", "Invalid opcode in Ring 3 User Mode. Terminating application.");
        maxp_close_all_windows();
        draw_window();
        if (ring3_is_app_active()) {
            exit_to_kernel();
        } else {
            task_exit();
        }
    }
}

void double_fault_handler(unsigned long long err, unsigned long long rip, unsigned long long cs) {
    debug_puts("[CPU] #DF DOUBLE FAULT! Fatal crash! ERR=0x");
    debug_print_num(err, 16);
    debug_puts(" RIP=0x");
    debug_print_num(rip, 16);
    debug_puts(" CS=0x");
    debug_print_num(cs, 16);
    debug_puts("\n");

    while (1) {
        __asm__ __volatile__("cli; hlt");
    }
}

void gp_fault_handler(unsigned long long err, unsigned long long rip, unsigned long long cs) {
    debug_puts("[CPU] #GP General Protection Fault! RIP=0x");
    debug_print_num(rip, 16);
    debug_puts(" CS=0x");
    debug_print_num(cs, 16);
    debug_puts(" ERR=0x");
    debug_print_num(err, 16);
    debug_puts("\n");

    if ((cs & 3) == 3) {
        debug_log("CPU", "Fault in Ring 3 User Mode. Gracefully terminating application.");
        maxp_close_all_windows();
        draw_window();
        if (ring3_is_app_active()) {
            exit_to_kernel();
        } else {
            task_exit();
        }
    }
}

void page_fault_handler(unsigned long long err, unsigned long long rip, unsigned long long cs) {
    unsigned long long cr2;
    __asm__ __volatile__("mov %%cr2, %0" : "=r"(cr2));
    debug_puts("[CPU] #PF Page Fault! CR2=0x");
    debug_print_num(cr2, 16);
    debug_puts(" RIP=0x");
    debug_print_num(rip, 16);
    debug_puts(" CS=0x");
    debug_print_num(cs, 16);
    debug_puts(" ERR=0x");
    debug_print_num(err, 16);
    debug_puts("\n");

    if ((cs & 3) == 3) {
        debug_log("CPU", "Fault in Ring 3 User Mode. Gracefully terminating application.");
        maxp_close_all_windows();
        draw_window();
        if (ring3_is_app_active()) {
            exit_to_kernel();
        } else {
            task_exit();
        }
    }
}

void idt_init(void) {
    // 1. Initialize all 256 gates with default exception handler
    for (int i = 0; i < 256; i++) {
        set_idt_gate(i, default_exception_entry, 0x8E); // Present, Ring 0, 64-bit Interrupt Gate
    }

    // 2. Install dedicated exception handlers
    set_idt_gate(0,  divide_error_entry, 0x8E);
    set_idt_gate(6,  invalid_opcode_entry, 0x8E);
    set_idt_gate(8,  double_fault_entry, 0x8E);
    set_idt_gate(13, gp_fault_entry, 0x8E);
    set_idt_gate(14, page_fault_entry, 0x8E);

    // 3. Install IRQ 0 (Timer Tick) at vector 32 (0x20)
    set_idt_gate(32, irq0_timer_entry, 0x8E);

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

#include "idt.h"
#include "kernel.h"

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

void idt_init(void) {
    // 1. Initialize all 256 gates with default exception handler
    for (int i = 0; i < 256; i++) {
        set_idt_gate(i, default_exception_entry, 0x8E); // Present, Ring 0, 64-bit Interrupt Gate
    }

    // 2. Install IRQ 0 (Timer Tick) at vector 32 (0x20)
    set_idt_gate(32, irq0_timer_entry, 0x8E);

    // 3. Load IDTR
    idtr.limit = sizeof(idt64) - 1;
    idtr.base = (unsigned long long)&idt64;
    load_idt(&idtr);

    // 4. Remap PIC and configure PIT to 1000 Hz (1ms per tick)
    pic_remap();
    pit_init(1000);

    // 5. Enable hardware interrupts!
    __asm__ __volatile__("sti");
}

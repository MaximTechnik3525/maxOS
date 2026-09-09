#ifndef IDT_H
#define IDT_H

// 64-bit Interrupt Descriptor Gate (16 bytes)
struct __attribute__((packed)) idt_entry_64 {
    unsigned short offset_low;    // Offset bits 0..15
    unsigned short selector;      // Target code segment selector (0x08)
    unsigned char  ist;           // Interrupt Stack Table index (bits 0..2)
    unsigned char  type_attr;     // Type and attribute flags (0x8E for Interrupt Gate)
    unsigned short offset_mid;    // Offset bits 16..31
    unsigned int   offset_high;   // Offset bits 32..63
    unsigned int   zero;          // Reserved (0)
};

// 64-bit IDTR Pointer structure
struct __attribute__((packed)) idt_ptr_64 {
    unsigned short limit;         // Size of IDT - 1
    unsigned long long base;      // Physical base address of IDT
};

// System Timer Tick Counter
extern volatile unsigned long long system_ticks;

// Subsystem Lifecycle
void idt_init(void);
void pic_remap(void);
void pit_init(unsigned int freq_hz);
unsigned long long get_uptime_ms(void);

// Assembly Routines
void load_idt(void* idtr);
void irq0_timer_entry(void);
void default_exception_entry(void);

#endif // IDT_H

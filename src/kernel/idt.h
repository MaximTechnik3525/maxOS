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

// Complete CPU trap frame for exception dispatcher
typedef struct __attribute__((packed)) {
    // Saved general-purpose registers (r15 down to rax)
    unsigned long long r15;
    unsigned long long r14;
    unsigned long long r13;
    unsigned long long r12;
    unsigned long long r11;
    unsigned long long r10;
    unsigned long long r9;
    unsigned long long r8;
    unsigned long long rbp;
    unsigned long long rdi;
    unsigned long long rsi;
    unsigned long long rdx;
    unsigned long long rcx;
    unsigned long long rbx;
    unsigned long long rax;

    // Pushed by specific ISR stub
    unsigned long long vector;
    unsigned long long error_code;

    // Pushed automatically by CPU on interrupt
    unsigned long long rip;
    unsigned long long cs;
    unsigned long long rflags;
    unsigned long long rsp;
    unsigned long long ss;
} isr_frame_t;

// System Timer Tick Counter
extern volatile unsigned long long system_ticks;

// Subsystem Lifecycle & Gate Configuration
void idt_init(void);
void idt_set_gate(int num, void* handler, unsigned char type_attr);
void pic_remap(void);
void pic_send_eoi(unsigned char irq);
void pic_set_mask(unsigned char irq);
void pic_clear_mask(unsigned char irq);
void pit_init(unsigned int freq_hz);
unsigned long long get_uptime_ms(void);

// Unified Exception & IRQ Dispatchers
void exception_dispatcher(isr_frame_t* frame);
void irq_default_handler(unsigned long long irq);

// Assembly Routines & Tables
void load_idt(void* idtr);
void irq0_timer_entry(void);

extern void* isr_stub_table[32];
extern void* irq_stub_table[16];

// Legacy entry points preserved for compatibility
void divide_error_entry(void);
void invalid_opcode_entry(void);
void double_fault_entry(void);
void gp_fault_entry(void);
void page_fault_entry(void);
void default_exception_entry(void);

#endif // IDT_H

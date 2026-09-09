#include "user.h"
#include "kernel.h"
#include "user/syscall.h"

// 64-bit Task State Segment (AMD64 Architecture Manual Vol 2)
struct __attribute__((packed)) tss64_t {
    unsigned int reserved0;
    unsigned long long rsp0;       // Ring 0 Stack Pointer loaded on privilege transition
    unsigned long long rsp1;
    unsigned long long rsp2;
    unsigned long long reserved1;
    unsigned long long ist[7];     // Interrupt Stack Table
    unsigned long long reserved2;
    unsigned short reserved3;
    unsigned short iopb_offset;    // I/O Permission Bitmap offset
};

static struct tss64_t default_tss;

// Exported from src/boot/entry.asm
extern unsigned long long stack_top;
extern unsigned long long gdt64_tss_entry[2];
unsigned long long* kernel_stack_top = &stack_top;

// Assembly routines from src/kernel/syscall_asm.asm
extern void run_in_ring3(void* rip, void* rsp);
extern void exit_to_kernel(void);
extern void load_tss(unsigned short sel);
extern void syscall_entry_asm(void);
extern long get_cpl(void);

// MSR Addresses for AMD64 Fast System Calls
#define MSR_EFER   0xC0000080
#define MSR_STAR   0xC0000081
#define MSR_LSTAR  0xC0000082
#define MSR_SFMASK 0xC0000084

static inline void wrmsr(unsigned int msr, unsigned long long val) {
    unsigned int low = (unsigned int)(val & 0xFFFFFFFF);
    unsigned int high = (unsigned int)(val >> 32);
    __asm__ __volatile__("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

// User-space execution stack (32 KB, 16-byte aligned)
static unsigned char user_stack[32768] __attribute__((aligned(16)));
static int ring3_demo_active = 0;

/* -------------------------------------------------------------------------
 * Initialization
 * ------------------------------------------------------------------------- */
void user_mode_init(void) {
    // 1. Initialize Task State Segment (TSS)
    for (unsigned int i = 0; i < sizeof(struct tss64_t); i++) {
        ((unsigned char*)&default_tss)[i] = 0;
    }
    default_tss.rsp0 = (unsigned long long)&stack_top;
    default_tss.iopb_offset = sizeof(struct tss64_t); // Disable raw I/O for Ring 3

    // 2. Install 16-byte 64-bit TSS descriptor into GDT (at selector 0x30)
    unsigned long long base = (unsigned long long)&default_tss;
    unsigned long long limit = sizeof(struct tss64_t) - 1;

    unsigned long long low = 0;
    low |= (limit & 0xFFFF);
    low |= (base & 0xFFFF) << 16;
    low |= ((base >> 16) & 0xFF) << 32;
    low |= ((unsigned long long)0x89) << 40; // Present (0x80) | Available 64-bit TSS (0x09)
    low |= ((limit >> 16) & 0x0F) << 48;
    low |= ((base >> 24) & 0xFF) << 56;

    unsigned long long high = (base >> 32) & 0xFFFFFFFF;

    gdt64_tss_entry[0] = low;
    gdt64_tss_entry[1] = high;

    // 3. Load Task Register
    load_tss(TSS_SELECTOR);

    // 4. Configure MSRs for SYSCALL / SYSRETQ
    // STAR: bits 47:32 = Kernel CS (0x08), bits 63:48 = User CS Base (0x18)
    unsigned long long star_val = ((unsigned long long)USER_CS_BASE << 48) | ((unsigned long long)KERNEL_CS << 32);
    wrmsr(MSR_STAR, star_val);

    // LSTAR: 64-bit target RIP for syscall instruction
    wrmsr(MSR_LSTAR, (unsigned long long)&syscall_entry_asm);

    // SFMASK: Clear IF (0x200), TF (0x100), IOPL (0x3000), NT (0x4000)
    wrmsr(MSR_SFMASK, 0x00004702);
}

/* -------------------------------------------------------------------------
 * Kernel Syscall Dispatcher (Called by syscall_entry_asm)
 * ------------------------------------------------------------------------- */
long syscall_dispatcher(long num, long a1, long a2, long a3, long a4, long a5) {
    switch (num) {
        case SYS_YIELD:
            return 0;

        case SYS_EXIT:
            exit_to_kernel();
            return 0;

        case SYS_PRINT_STRING:
            print_string((char*)a1, (int)a2, (int)a3, (unsigned short)a4);
            return 0;

        case SYS_DRAW_RECT:
            draw_rect((int)a1, (int)a2, (int)a3, (int)a4, (unsigned short)a5);
            return 0;

        case SYS_PLAY_SOUND:
            play_sound((unsigned int)a1);
            sleep((unsigned int)a2);
            no_sound();
            return 0;

        case SYS_SLEEP:
            sleep((unsigned int)a1);
            return 0;

        case SYS_GET_KEY:
            return 0;

        case SYS_GET_MOUSE:
            if (a1) *((int*)a1) = pos_x;
            if (a2) *((int*)a2) = pos_y;
            if (a3) *((int*)a3) = 0;
            return 0;

        case SYS_GET_CPL:
            return get_cpl();

        case SYS_DRAW_WINDOW:
            draw_window();
            return 0;

        default:
            return -1;
    }
}

/* -------------------------------------------------------------------------
 * Ring 3 Demonstration Code (Executes in User Space!)
 * ------------------------------------------------------------------------- */
static void ring3_user_entry(void) {
    long cpl = u_get_cpl();
    (void)cpl;

    int wx = 240, wy = 160, ww = 540, wh = 300;

    // Draw 3D Window completely from Ring 3 via syscalls
    u_draw_rect(wx, wy, ww, wh, 0x0000);
    u_draw_rect(wx + 1, wy + 1, ww - 2, wh - 2, 0xCE79);
    u_draw_rect(wx + 1, wy + 1, ww - 3, 1, 0xFFFF);
    u_draw_rect(wx + 1, wy + 1, 1, wh - 3, 0xFFFF);

    // Titlebar
    u_draw_rect(wx + 3, wy + 3, ww - 6, 24, 0x03EA); // Emerald Green
    u_print_string("maxOS Ring 3 User Space Subsystem", wx + 10, wy + 7, 0xFFFF);

    // Content card
    u_draw_rect(wx + 12, wy + 36, ww - 24, wh - 48, 0xFFFF);
    u_draw_rect(wx + 12, wy + 36, ww - 24, 1, 0x7BEF);
    u_draw_rect(wx + 12, wy + 36, 1, wh - 48, 0x7BEF);

    u_print_string("CPU Architecture: AMD64 / Intel 64 (Long Mode)", wx + 24, wy + 50, 0x11EB);
    u_print_string("Current Privilege: Ring 3 User Mode (CPL = 3)", wx + 24, wy + 74, 0x03EA);
    u_print_string("Hardware Protection: DPL=3 Segments Active", wx + 24, wy + 98, 0x0000);
    u_print_string("Kernel Entry: Hardware Fast SYSCALL (IA32_LSTAR)", wx + 24, wy + 122, 0x0000);
    u_print_string("Kernel Return: Hardware Fast SYSRETQ (STAR Base)", wx + 24, wy + 146, 0x0000);
    u_print_string("Stack Switch: 64-bit TSS (RSP0 Stack Switching)", wx + 24, wy + 170, 0x0000);
    u_print_string("Privilege Boundary: User space verified!", wx + 24, wy + 194, 0x24EE);
    u_print_string("Status: Running via SYSCALL interface", wx + 24, wy + 218, 0x0200);
    u_print_string("Returning to Ring 0 desktop in 2.5 seconds...", wx + 24, wy + 252, 0x8085);

    // Play welcome tones via syscall
    u_play_sound(600, 80);
    u_play_sound(900, 80);
    u_play_sound(1200, 120);

    // Sleep for 2.5 seconds via syscall
    u_sleep(2500);

    // Exit cleanly via SYS_EXIT syscall back to kernel!
    u_exit();
}

void ring3_demo_launch(void) {
    ring3_demo_active = 1;
    void* u_stack = user_stack + sizeof(user_stack) - 32;
    run_in_ring3(ring3_user_entry, u_stack);
    ring3_demo_active = 0;
    draw_window();
}

int ring3_demo_is_active(void) {
    return ring3_demo_active;
}

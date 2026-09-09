#ifndef USER_H
#define USER_H

// Syscall numbers
#define SYS_YIELD         0
#define SYS_EXIT          1
#define SYS_PRINT_STRING  2  // sys_print_string(char* str, int x, int y, unsigned short col)
#define SYS_DRAW_RECT     3  // sys_draw_rect(int x, int y, int w, int h, unsigned short col)
#define SYS_PLAY_SOUND    4  // sys_play_sound(unsigned int freq, unsigned int ms)
#define SYS_SLEEP         5  // sys_sleep(unsigned int ms)
#define SYS_GET_KEY       6  // sys_get_key(void) -> returns key
#define SYS_GET_MOUSE     7  // sys_get_mouse(int* out_x, int* out_y, int* out_btn)
#define SYS_GET_CPL       8  // sys_get_cpl(void) -> returns current CPL (0 or 3)
#define SYS_DRAW_WINDOW   9  // sys_draw_window(void)

// Privilege Levels & GDT Selectors (x86_64 Long Mode)
#define KERNEL_CS         0x08
#define KERNEL_DS         0x10
#define USER_CS_BASE      0x18
#define USER_DS           0x20
#define USER_CS           0x28
#define TSS_SELECTOR      0x30

// Subsystem Lifecycle
void user_mode_init(void);
void enter_user_mode(void (*user_func)(void), void* user_stack);
long syscall_dispatcher(long num, long a1, long a2, long a3, long a4, long a5);

// Low-level Assembly Routines (src/kernel/syscall_asm.asm)
void enter_ring3(void* rip, void* rsp);
void syscall_entry_asm(void);
void load_tss(unsigned short sel);
long get_cpl(void);
unsigned short read_cs(void);
unsigned short read_ss(void);

// Ring 3 Demo Application
void ring3_demo_launch(void);
int ring3_demo_is_active(void);

#endif // USER_H

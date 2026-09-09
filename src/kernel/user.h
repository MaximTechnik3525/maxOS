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
#define SYS_GET_TICKS     10 // sys_get_ticks(void) -> returns millisecond tick count
#define SYS_NO_SOUND      11 // sys_no_sound(void)
#define SYS_ATA_READ      12 // sys_ata_read(unsigned int lba, unsigned char* buf)
#define SYS_ATA_WRITE     13 // sys_ata_write(unsigned int lba, const unsigned char* buf)
#define SYS_ATA_FLUSH     14 // sys_ata_flush(void)
#define SYS_ATA_STATUS    15 // sys_ata_status(void)
#define SYS_DEBUG_LOG     16 // sys_debug_log(char* tag, char* msg)
#define SYS_SPAWN         17 // sys_spawn(char* name, void* entry, int is_user) -> pid
#define SYS_KILL          18 // sys_kill(int pid) -> status
#define SYS_GETPID        19 // sys_getpid(void) -> pid
#define SYS_TASKLIST      20 // sys_tasklist(void* buf, int max) -> count

// Privilege Levels & GDT Selectors (x86_64 Long Mode)
#define KERNEL_CS         0x08
#define KERNEL_DS         0x10
#define USER_CS_BASE      0x18
#define USER_DS           0x20
#define USER_CS           0x28
#define TSS_SELECTOR      0x30

// Subsystem Lifecycle
void user_mode_init(void);
void tss_set_rsp0(unsigned long long rsp0);
unsigned long long tss_get_rsp0(void);
void enter_user_mode(void (*user_func)(void), void* user_stack);
long syscall_dispatcher(long num, long a1, long a2, long a3, long a4, long a5);

// Low-level Assembly Routines (src/kernel/syscall_asm.asm)
void enter_ring3(void* rip, void* rsp);
void run_in_ring3(void* rip, void* rsp);
void exit_to_kernel(void);
void syscall_entry_asm(void);
void load_tss(unsigned short sel);
long get_cpl(void);
unsigned short read_cs(void);
unsigned short read_ss(void);

// Ring 3 Application Runner (Executes any app in Ring 3 User Space)
void ring3_app_draw(int app_id);
int ring3_app_handle_click(int app_id, int mouse_x, int mouse_y);
int ring3_app_handle_key(int app_id, char ascii_char, unsigned char scan_code);
void ring3_app_step(int app_id);
int ring3_is_app_active(void);

// Ring 3 Demo Application
void ring3_demo_launch(void);
int ring3_demo_is_active(void);

#endif // USER_H

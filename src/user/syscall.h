#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

// Syscall numbers
#define SYS_YIELD         0
#define SYS_EXIT          1
#define SYS_PRINT_STRING  2
#define SYS_DRAW_RECT     3
#define SYS_PLAY_SOUND    4
#define SYS_SLEEP         5
#define SYS_GET_KEY       6
#define SYS_GET_MOUSE     7
#define SYS_GET_CPL       8
#define SYS_DRAW_WINDOW   9
#define SYS_GET_TICKS     10
#define SYS_NO_SOUND      11
#define SYS_ATA_READ      12
#define SYS_ATA_WRITE     13
#define SYS_ATA_FLUSH     14
#define SYS_ATA_STATUS    15
#define SYS_DEBUG_LOG     16

// Low-level 64-bit SYSCALL wrappers (System V AMD64 ABI)
static inline long syscall0(long num) {
    long ret;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall1(long num, long a1) {
    long ret;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall2(long num, long a1, long a2) {
    long ret;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall3(long num, long a1, long a2, long a3) {
    long ret;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall4(long num, long a1, long a2, long a3, long a4) {
    long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall5(long num, long a1, long a2, long a3, long a4, long a5) {
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8  __asm__("r8")  = a5;
    __asm__ __volatile__ (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// Client helper functions for user space
static inline void u_exit(void) {
    syscall0(SYS_EXIT);
}

static inline void u_print_string(const char* s, int x, int y, unsigned short col) {
    syscall4(SYS_PRINT_STRING, (long)s, (long)x, (long)y, (long)col);
}

static inline void u_draw_rect(int x, int y, int w, int h, unsigned short col) {
    syscall5(SYS_DRAW_RECT, (long)x, (long)y, (long)w, (long)h, (long)col);
}

static inline void u_play_sound(unsigned int freq, unsigned int ms) {
    syscall2(SYS_PLAY_SOUND, (long)freq, (long)ms);
}

static inline void u_sleep(unsigned int ms) {
    syscall1(SYS_SLEEP, (long)ms);
}

static inline long u_get_cpl(void) {
    return syscall0(SYS_GET_CPL);
}

static inline void u_draw_window(void) {
    syscall0(SYS_DRAW_WINDOW);
}

static inline unsigned long long u_get_ticks(void) {
    return (unsigned long long)syscall0(SYS_GET_TICKS);
}

static inline void u_no_sound(void) {
    syscall0(SYS_NO_SOUND);
}

static inline int u_ata_read(unsigned int lba, unsigned char* buf) {
    return (int)syscall2(SYS_ATA_READ, (long)lba, (long)buf);
}

static inline int u_ata_write(unsigned int lba, const unsigned char* buf) {
    return (int)syscall2(SYS_ATA_WRITE, (long)lba, (long)buf);
}

static inline int u_ata_flush(void) {
    return (int)syscall0(SYS_ATA_FLUSH);
}

static inline int u_ata_status(void) {
    return (int)syscall0(SYS_ATA_STATUS);
}

static inline void u_debug_log(const char* tag, const char* msg) {
    syscall2(SYS_DEBUG_LOG, (long)tag, (long)msg);
}

#endif // USER_SYSCALL_H

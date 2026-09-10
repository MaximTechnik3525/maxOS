// Упакованная структура, чтобы компилятор не добавлял лишних скрытых байт (alignment padding)
#pragma pack(push, 1)
struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];       // Поля для ELF секций
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;

    // Графический фреймбуфер (начиная с 88-го байта структуры)
    unsigned long long framebuffer_addr; 
    unsigned int framebuffer_pitch;     // Длина строки в БАЙТАХ
    unsigned int framebuffer_width;     // Ширина экрана в пикселях
    unsigned int framebuffer_height;    // Высота экрана в пикселях
    unsigned char framebuffer_bpp;       // Количество бит на пиксель (16, 24 или 32)
    unsigned char framebuffer_type;
    unsigned char framebuffer_color_info[6];
};
#pragma pack(pop)

#include "maxfs.h"
#include "notepad.h"
#include "installer.h"
#include "explorer.h"
#include "ata.h"
#include "maxp.h"
#include "taskbar.h"
#include "calc.h"
#include "sysinfo.h"
#include "pong.h"
#include "mem.h"
#include "kernel.h"
#include "mmu.h"
#include "pmm.h"
#include "malloc.h"
#include "user.h"
#include "idt.h"
#include "task.h"
#include "debug.h"
#include "user/syscall.h"
#include "font.h"
#include "string.h"

extern char _kernel_start[];
extern char _kernel_end[];

static unsigned int mb_mem_lower = 640;
static unsigned int mb_mem_upper = 130048;

unsigned short* _gfx_memory_backend;
unsigned int REAL_PITCH = 1024;
#define gfx_memory_safe(y, x) _gfx_memory_backend[(y) * REAL_PITCH + (x)]

void get_cpu(char* buffer);
void wait_mouse(unsigned char type);
void init_mouse();
void pong(void);
void help(void);
void cpu_win(void);
void filew(void);
unsigned short bg_col = 0x18C3;

int win_x = 150;
int win_y = 140;
int win_w = 740;
int win_h = 550;
int pos_x = 512;
int pos_y = 384;
unsigned short cursor_bg[12][12];
int cursor_bg_saved = 0;
int cursor_saved_x = -1;
int cursor_saved_y = -1;
int theme = 5;
int w_mode = 0;
int km_mode = 0;
int drag = 2;
int tail = 0;
int corners = 0;

static int mouse_cycle = 0;
static unsigned char mouse_packet[3];
static int mouse_accum_x = 0;
static int mouse_accum_y = 0;
static int prev_mouse_click = 0;
static int in_sleep_poll = 0;

static void handle_mouse_packet(unsigned char p0, unsigned char p1, unsigned char p2) {
    int sign_x = p0 & 0x10;
    int sign_y = p0 & 0x20;
    int click = p0 & 0x01;
    int delta_x = p1;
    int delta_y = p2;
    if (sign_x) delta_x |= 0xFFFFFF00;
    if (sign_y) delta_y |= 0xFFFFFF00;

    if (delta_x < -250 || delta_x > 250 || delta_y < -250 || delta_y > 250) {
        return;
    }

    // Accumulator ensures smooth 1:1 mouse movement without dropping slow movements
    mouse_accum_x += delta_x;
    mouse_accum_y -= delta_y;

    int move_x = mouse_accum_x / 2;
    int move_y = mouse_accum_y / 2;

    if (move_x != 0 || move_y != 0) {
        mouse_accum_x %= 2;
        mouse_accum_y %= 2;

        prev_cursor();
        pos_x += move_x;
        pos_y += move_y;
        if (pos_x > 1024 - 12) pos_x = 1024 - 12;
        if (pos_x < 0) pos_x = 0;
        if (pos_y > 768 - 12) pos_y = 768 - 12;
        if (pos_y < 0) pos_y = 0;
        draw_cursor(pos_x, pos_y);
    }

    // Edge-triggered click: fires once when button transitions from unpressed to pressed
    int mouse_down = (click == 1 && prev_mouse_click == 0);
    prev_mouse_click = click;

    if (in_sleep_poll) {
        return;
    }

    if (mouse_down) {
        if (taskbar_handle_click(pos_x, pos_y)) {
            return;
        }
        int active_app = maxp_get_active_app();
        int handled = 0;
        if (active_app != MAXP_APP_NONE) {
            handled = ring3_app_handle_click(active_app, pos_x, pos_y);
            if (handled == -3) {
                ring3_demo_launch();
                handled = 1;
            }
        }
        if (!handled) {
            handled = desktop_handle_click(pos_x, pos_y);
        }
    }
}

static inline void process_mouse_byte(unsigned char b) {
    if (mouse_cycle == 0) {
        if ((b & 0x08) && !(b & 0xC0)) {
            mouse_packet[0] = b;
            mouse_cycle = 1;
        }
    } else if (mouse_cycle == 1) {
        mouse_packet[1] = b;
        mouse_cycle = 2;
    } else if (mouse_cycle == 2) {
        mouse_packet[2] = b;
        mouse_cycle = 0;
        handle_mouse_packet(mouse_packet[0], mouse_packet[1], mouse_packet[2]);
    }
}

void kmain(unsigned long multiboot_info_address, unsigned long magic) {
    (void)magic;
    struct multiboot_info* mbi = (struct multiboot_info*) multiboot_info_address;
    _gfx_memory_backend = (unsigned short*)(unsigned long)mbi->framebuffer_addr;
    if (mbi->framebuffer_pitch > 0) { REAL_PITCH = mbi->framebuffer_pitch / 2; }
    if (mbi->flags & 0x01) {
        mb_mem_lower = mbi->mem_lower;
        mb_mem_upper = mbi->mem_upper;
    }

    // Initialize Diagnostic Serial Debugger (COM1 38400 baud)
    debug_init();
    
    // Initialize Memory Management Unit (4KB pages & Security)
    mmu_init();
    
    // Initialize Physical Memory Manager and Kernel Heap
    pmm_init(mb_mem_upper);
    malloc_init();

    init_mouse();

    // Initialize 64-bit IDT, 8259 PIC Remap, and PIT 1000Hz Timer
    idt_init();

    // Initialize Ring 3 User Space, TSS, and SYSCALL MSRs
    user_mode_init();

    // Initialize Preemptive Task Scheduler (Ring 0 & Ring 3 Multitasking)
    task_init();

    // Splash screen
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 387 && y >= 384) {
                gfx_memory[y * 1024 + x] = 0x0DE5;
            } else if (y <= 393 && y > 387) {
                gfx_memory[y * 1024 + x] = 0x03EA;
            } else if (y <= 400 && y > 393) {
                gfx_memory[y * 1024 + x] = 0x01A4;
            } else {
                gfx_memory[y * 1024 + x] = 0x0000;
            }
        }
    }
    print_string("maxOS MaxRing v3.5 x86_64", 380, 420, 0x0DE5);
    print_string("by maxTech", 10, 10, 0x24EE);
    play_sound(100); sleep(150); play_sound(200); sleep(150); play_sound(400); sleep(150); play_sound(600); sleep(150); play_sound(50); sleep(200); no_sound();
    sleep(1500);

    // Initialize all filesystem, application, and GUI subsystems
    maxfs_init();
    maxp_init();
    taskbar_init();
    notepad_init();
    installer_init();
    explorer_init();
    calc_init();
    sysinfo_init();
    pong_init();
    mem_init();

    drag = 0;
    draw_window();

    int clock_timer = 0;

    while (1) {
        unsigned char status = inb(0x64);
        if (status & 0x01) {
            if ((status & 0x20) && w_mode == 0 && drag != 2) {
                // PS/2 Mouse byte (completely non-blocking, zero timeouts)
                process_mouse_byte(inb(0x60));
            } else {
                // PS/2 Keyboard scancode
                unsigned char scan_code = inb(0x60);
                if (scan_code < 0x80 && drag != 2) {
                    char ascii_char = scan_code_to_ascii(scan_code);

                    if (taskbar_handle_key(ascii_char, scan_code)) {
                        continue;
                    }

                    // Global hotkey: F1 (0x3B) or Win key (0x5B, 0x5C) toggles Start Menu
                    if (scan_code == 0x3B || scan_code == 0x5B || scan_code == 0x5C) {
                        taskbar_toggle_start_menu();
                        continue;
                    }

                    // Global hotkey: Tab (0x0F) cycles through running instances (Alt-Tab style!)
                    if (scan_code == 0x0F) {
                        if (maxp_get_instance_count() > 0) {
                            maxp_cycle_active_instance();
                            taskbar_set_app_minimized(0);
                            play_sound(750); sleep(20); no_sound();
                            draw_window();
                            continue;
                        }
                    }

                    int active_app = maxp_get_active_app();
                    if (active_app != MAXP_APP_NONE) {
                        int res = ring3_app_handle_key(active_app, ascii_char, scan_code);
                        if (res == -3) {
                            ring3_demo_launch();
                            continue;
                        } else if (res) {
                            continue;
                        }
                    }

                    // Start Menu shortcut (M key on desktop)
                    if (ascii_char == 'm' || ascii_char == 'M') {
                        taskbar_toggle_start_menu();
                        continue;
                    }

                    // Close active window instance (or all if desktop)
                    if (ascii_char == 'c' || ascii_char == 'C' || scan_code == 0x01) {
                        int act_id = maxp_get_active_instance_id();
                        if (act_id > 0) {
                            maxp_close_instance(act_id);
                        } else {
                            maxp_close_all_windows();
                            draw_window();
                        }
                        continue;
                    }

                    // Hotkeys ONLY when no application window is active (prevents accidental launches & theme switches!)
                    if (maxp_get_instance_count() == 0 || taskbar_is_app_minimized()) {
                        if (ascii_char == 'F' || ascii_char == 'f') { maxp_launch_app(MAXP_APP_NOTEPAD); continue; }
                        if (ascii_char == 'E' || ascii_char == 'e') { maxp_launch_app(MAXP_APP_EXPLORER); continue; }
                        if (ascii_char == 'K' || ascii_char == 'k') { maxp_launch_app(MAXP_APP_CALC); continue; }
                        if (ascii_char == 'S' || ascii_char == 's') { maxp_launch_app(MAXP_APP_SYSINFO); continue; }
                        if (ascii_char == 'R' || ascii_char == 'r') { maxp_launch_app(MAXP_APP_MEM); continue; }
                        if (ascii_char == 'P' || ascii_char == 'p') { maxp_launch_app(MAXP_APP_PONG); continue; }
                        if (ascii_char == 'I' || ascii_char == 'i') { maxp_launch_app(MAXP_APP_INSTALLER); continue; }
                        if (ascii_char == 'U' || ascii_char == 'u') { ring3_demo_launch(); continue; }

                        // Themes 1-9 on desktop
                        if (ascii_char >= '1' && ascii_char <= '9') {
                            theme = ascii_char - '0';
                            if (theme == 1) bg_col = 0x18C3;
                            else if (theme == 2) bg_col = 0x2000;
                            else if (theme == 3) bg_col = 0x1041;
                            else if (theme == 4) bg_col = 0x10A2;
                            else if (theme == 5) bg_col = 0x01C8;
                            else if (theme == 6) bg_col = 0x00A1;
                            else if (theme == 7) bg_col = 0x4083;
                            else if (theme == 8) bg_col = 0x7BE0;
                            else if (theme == 9) bg_col = 0x0110;
                            draw_window();
                            play_sound(700); sleep(30); no_sound();
                            continue;
                        }
                    }

                    // Rounded corners toggle
                    if (ascii_char == 'M') { corners = 1; draw_window(); play_sound(700); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'N') { corners = 0; draw_window(); play_sound(500); sleep(50); no_sound(); continue; }

                    // Mouse trail toggle
                    if (ascii_char == 'C' && tail == 0) { tail = 1; play_sound(900); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'O' && tail == 1) { tail = 0; draw_window(); play_sound(800); sleep(50); no_sound(); continue; }

                    // Key-mouse mode
                    if (ascii_char == 'T') { km_mode = 1; play_sound(200); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'G') { km_mode = 0; play_sound(1000); sleep(50); no_sound(); draw_window(); continue; }

                    if (km_mode == 1) {
                        if (ascii_char == 'U' && pos_y >= 15) { prev_cursor(); pos_y -= 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'D' && pos_y <= 768 - 27) { prev_cursor(); pos_y += 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'R' && pos_x <= 1024 - 27) { prev_cursor(); pos_x += 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'L' && pos_x >= 15) { prev_cursor(); pos_x -= 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'e') {
                            if (taskbar_handle_click(pos_x, pos_y)) continue;
                            if (notepad_open && notepad_handle_click(pos_x, pos_y)) continue;
                            if (explorer_open && explorer_handle_click(pos_x, pos_y)) continue;
                            if (calc_open && calc_handle_click(pos_x, pos_y)) continue;
                            if (sysinfo_open && sysinfo_handle_click(pos_x, pos_y)) continue;
                            if (mem_open && mem_handle_click(pos_x, pos_y)) continue;
                            if (pong_open && pong_handle_click(pos_x, pos_y)) continue;
                            if (installer_open && installer_handle_click(pos_x, pos_y)) continue;
                        }
                    }

                    // Format disk
                    if (ascii_char == 'f' && drag == 0) {
                        maxfs_format("maxOS Disk");
                        play_sound(800); sleep(100); no_sound();
                    }
                }
            }
        } else {
            // Non-blocking tick dispatch for animated apps (Pong, Mem stress)
            static unsigned long long last_step_tick = 0;
            if (maxp_has_ticking_instances()) {
                if (system_ticks - last_step_tick >= 20) {
                    last_step_tick = system_ticks;
                    ring3_app_step(maxp_get_active_app());
                }
            }

            clock_timer++;
            if (clock_timer >= 100000) {
                clock_timer = 0;
                taskbar_draw_clock();
            }
            __asm__ __volatile__("pause");
        }
    }
}

void pong(void) {
    maxp_launch_app(MAXP_APP_PONG);
}

void filew(void) {
    maxp_launch_app(MAXP_APP_EXPLORER);
}

void help(void) {
    notepad_open_file_by_name("readme.txt");
    maxp_set_active_app(MAXP_APP_NOTEPAD);
}

void cpu_win(void) {
    maxp_launch_app(MAXP_APP_SYSINFO);
}

void int_str(int num, char* str) {
    int_to_str(num, str);
}

void get_system_mem_info(struct SystemMemInfo* info) {
    if (!info) return;

    info->lower_kb = mb_mem_lower;
    info->upper_kb = mb_mem_upper;

    // Total memory: 1024 KB conventional + upper_kb extended memory
    if (mb_mem_upper > 0) {
        info->total_kb = 1024 + mb_mem_upper;
    } else {
        info->total_kb = 128 * 1024;
    }

    // Kernel binary size in RAM
    unsigned long k_size = (unsigned long)(_kernel_end - _kernel_start);
    info->kernel_kb = (unsigned int)((k_size + 1023) / 1024);

    // VESA VBE linear framebuffer: 1024 * 768 * 2 = 1,572,864 bytes = 1536 KB
    info->vram_kb = (SCREEN_WIDTH * SCREEN_HEIGHT * 2) / 1024;

    // 64-bit Paging structures: PML4 (4KB) + PDPT (4KB) + 4xPD (16KB) = 24 KB
    info->paging_kb = 24;

    // Stacks: Kernel stack (16KB) + Ring 3 User stack (16KB) + TSS (4KB) = 36 KB
    info->stacks_kb = 36;

    // maxFS storage: 32 files * 1064 bytes + metadata = ~38 KB
    info->ramdisk_kb = 38;

    // Dynamic app heap & active buffers: 1024 KB base + 256 KB per open app
    unsigned int active_apps_count = 0;
    if (notepad_open) active_apps_count++;
    if (explorer_open) active_apps_count++;
    if (calc_open) active_apps_count++;
    if (sysinfo_open) active_apps_count++;
    if (pong_open) active_apps_count++;
    if (installer_open) active_apps_count++;
    if (mem_open) active_apps_count++;

    info->apps_dynamic_kb = 1024 + (active_apps_count * 256);
    info->stress_kb = mem_get_stress_kb();

    info->used_kb = info->kernel_kb + info->vram_kb + info->paging_kb +
                    info->stacks_kb + info->ramdisk_kb + info->apps_dynamic_kb +
                    info->stress_kb;

    if (info->used_kb > info->total_kb) {
        info->free_kb = 0;
        info->usage_percent = 100;
    } else {
        info->free_kb = info->total_kb - info->used_kb;
        info->usage_percent = (unsigned int)(((unsigned long long)info->used_kb * 100) / info->total_kb);
        if (info->usage_percent == 0 && info->used_kb > 0) info->usage_percent = 1;
    }
}
void pump_events_nonblocking(void) {
    while (inb(0x64) & 0x01) {
        unsigned char status = inb(0x64);
        if ((status & 0x20) && w_mode == 0 && drag != 2) {
            process_mouse_byte(inb(0x60));
        } else {
            break; // Leave keyboard scancodes for main event loop
        }
    }
}

void sleep(unsigned int ms) {
    if (get_cpl() == 3) {
        u_sleep(ms);
        return;
    }
    unsigned long long start_ticks = system_ticks;
    unsigned long long target = start_ticks + ms;
    int safety = 5000000;
    in_sleep_poll = 1;
    while (system_ticks < target && safety--) {
        pump_events_nonblocking();
        __asm__ __volatile__("pause");
    }
    in_sleep_poll = 0;
    if (system_ticks == start_ticks) {
        for (unsigned int i = 0; i < ms; i++) {
            outb(0x43, 0x30);
            outb(0x40, 0xA9);
            outb(0x40, 0x04);
            unsigned char status = 0;
            while ((status & 0x80) == 0) {
                outb(0x43, 0xE2);
                status = inb(0x40);
            }
        }
    }
}
void outw(unsigned short port, unsigned short val) {
    if (get_cpl() == 3) return;
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
void system_reboot() {
    unsigned char temp;
    do {
        temp = inb(0x64);
        if (temp & 1) inb(0x60);
    } while (temp & 2);
    outb(0x64, 0xFE);
    while (1) { __asm__ volatile("hlt"); }
}
void shutdown() {
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 387 && y >= 384) {
                gfx_memory[y * 1024 + x] = 0xB269;
            }
            else if (y <= 393 && y > 387) {
                gfx_memory[y * 1024 + x] = 0x81C6;
            }
            else if (y <= 400 && y > 393) {
                gfx_memory[y * 1024 + x] = 0x4924;
            }
            else { gfx_memory[y * 1024 + x] = 0x0000; }

        }
    }
    print_string("maxOS is shutting down...", 420, 420, 0xB269);
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(150); no_sound();
    play_sound(50); sleep(250); no_sound();
    sleep(3000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x0B004, 0x2000);
    error("Cannot use power off ports! Code: 0x01");
    asm volatile("cli; hlt");
}
void error(char* err) {
    drag = 2;
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            gfx_memory[y * 1024 + x] = 0xB269;
        }
    }
    print_string("maxOS error!", 470, 10, 0xFFFF);
    print_string(err, 10, 30, 0xFFFF);
    print_string("Restart or off your PC :(", 10, 50, 0xFFFF);
    play_sound(100); sleep(250); play_sound(75); sleep(250); play_sound(50); sleep(250); no_sound();
}
int str_in(char* main_string, char* substring) {
    int i = 0;
    if (substring[0] == '\0') return 1;
    while (main_string[i] != '\0') {
        int j = 0;
        while (main_string[i + j] == substring[j] && substring[j] != '\0') { j++; }
        if (substring[j] == '\0') { return 1; }
        i++;
    }
    return 0;
}
void play_sound(unsigned int nfreq) {
    if (get_cpl() == 3) {
        u_play_sound(nfreq, 0);
        return;
    }
    if (nfreq == 0) return;
    unsigned int div;
    unsigned char tmp;
    div = 1193180 / nfreq;
    outb(0x43, 0xB6);
    outb(0x42, (unsigned char) (div & 0xFF));
    outb(0x42, (unsigned char) ((div >> 8) & 0xFF));
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}
void no_sound() {
    if (get_cpl() == 3) {
        u_no_sound();
        return;
    }
    unsigned char tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}
void prev_cursor() {
    if (!cursor_bg_saved) return;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (mouse_arrow[y][x] != 0) {
                int cur_x = cursor_saved_x + x;
                int cur_y = cursor_saved_y + y;
                if (cur_x >= 0 && cur_x < 1024 && cur_y >= 0 && cur_y < 768) {
                    gfx_memory[cur_y * 1024 + cur_x] = cursor_bg[y][x];
                }
            }
        }
    }
    cursor_bg_saved = 0;
    cursor_saved_x = -1;
    cursor_saved_y = -1;
}
void outb(unsigned short port, unsigned char data) {
    if (get_cpl() == 3) return;
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}
void wait_mouse(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
    else {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    }
}
void init_mouse() {
    wait_mouse(0);
    outb(0x64, 0xA8);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0xF4);
    wait_mouse(1);
    inb(0x60);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0xE8);
    wait_mouse(1);
    inb(0x60);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0x00);
    wait_mouse(1);
    inb(0x60);
}

void get_cpu(char* buffer) {
    unsigned int registers[4];
    for (unsigned int i = 0; i < 3; i++) {
        unsigned int id = 0x80000002 + i;
        __asm__ __volatile__ ("cpuid" : "=a"(registers[0]), "=b"(registers[1]), "=c"(registers[2]), "=d"(registers[3]) : "a"(id));
    
        for (int reg = 0; reg < 4; reg++) {
            for (int byte = 0; byte < 4; byte++) {
                *buffer = (registers[reg] >> (byte * 8)) & 0xFF;
                buffer++;
            }
        }
    }
    *buffer = 0;
}

void draw_rect(int rx, int ry, int rw, int rh, unsigned short color) {
    if (get_cpl() == 3) {
        u_draw_rect(rx, ry, rw, rh, color);
        return;
    }
    if (rx < 0) { rw += rx; rx = 0; }
    if (ry < 0) { rh += ry; ry = 0; }
    if (rx + rw > SCREEN_WIDTH) rw = SCREEN_WIDTH - rx;
    if (ry + rh > SCREEN_HEIGHT) rh = SCREEN_HEIGHT - ry;
    if (rw <= 0 || rh <= 0) return;

    if (cursor_bg_saved) {
        int cx0 = cursor_saved_x, cy0 = cursor_saved_y;
        int cx1 = cx0 + 12, cy1 = cy0 + 12;
        int ix0 = (rx > cx0) ? rx : cx0;
        int iy0 = (ry > cy0) ? ry : cy0;
        int ix1 = (rx + rw < cx1) ? (rx + rw) : cx1;
        int iy1 = (ry + rh < cy1) ? (ry + rh) : cy1;
        if (ix0 < ix1 && iy0 < iy1) {
            for (int cy = iy0; cy < iy1; cy++) {
                int by = cy - cy0;
                for (int cx = ix0; cx < ix1; cx++) {
                    int bx = cx - cx0;
                    cursor_bg[by][bx] = color;
                }
            }
        }
    }

    for (int y = ry; y < ry + rh; y++) {
        int row = y * SCREEN_WIDTH;
        for (int x = rx; x < rx + rw; x++) {
            if (cursor_bg_saved &&
                x >= cursor_saved_x && x < cursor_saved_x + 12 &&
                y >= cursor_saved_y && y < cursor_saved_y + 12 &&
                mouse_arrow[y - cursor_saved_y][x - cursor_saved_x] != 0) {
                continue;
            }
            gfx_memory[row + x] = color;
        }
    }
}
void draw_cursor(int mouse_x, int mouse_y) {
    if (cursor_bg_saved) {
        prev_cursor();
    }
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x > 1024 - 12) mouse_x = 1024 - 12;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y > 768 - 12) mouse_y = 768 - 12;

    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x >= 0 && screen_x < 1024 && screen_y >= 0 && screen_y < 768) {
                cursor_bg[y][x] = gfx_memory[screen_y * 1024 + screen_x];
            } else {
                cursor_bg[y][x] = 0x0000;
            }
        }
    }
    cursor_saved_x = mouse_x;
    cursor_saved_y = mouse_y;
    cursor_bg_saved = 1;

    static const struct {
        unsigned short c1, c2;
    } theme_cursors[10] = {
        { 0x0000, 0xFFFF }, // 0
        { 0x0000, 0xFFFF }, // 1: Classic
        { 0x4000, 0xF800 }, // 2: Crimson
        { 0x4080, 0xB269 }, // 3: Forest
        { 0x3186, 0xD69F }, // 4: Royal Blue
        { 0x0168, 0x07FF }, // 5: Cyan
        { 0x0200, 0x07E0 }, // 6: Emerald
        { 0x60A4, 0xFDF3 }, // 7: Rose
        { 0x4962, 0xF621 }, // 8: Amber
        { 0x5000, 0xFBE0 }  // 9: Obsidian
    };
    int t_cur = (theme >= 1 && theme <= 9) ? theme : 1;
    unsigned short cur_c1 = theme_cursors[t_cur].c1;
    unsigned short cur_c2 = theme_cursors[t_cur].c2;

    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            unsigned char pixel_type = mouse_arrow[y][x];
            if (pixel_type == 0) continue;
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x >= 0 && screen_x < 1024 && screen_y >= 0 && screen_y < 768) {
                if (pixel_type == 1) { gfx_memory[screen_y * 1024 + screen_x] = cur_c1; }
                else if (pixel_type == 2) { gfx_memory[screen_y * 1024 + screen_x] = cur_c2; }
                else if (pixel_type == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3; }
            }
        }
    }
}

unsigned char inb(unsigned short port) {
    if (get_cpl() == 3) return 0;
    unsigned char result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

char scan_code_to_ascii(unsigned char scan_code) {
    switch (scan_code) {
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        case 0x3B: return 'F';
        case 0x3C: return 'S';
        case 0x3D: return 'T';
        case 0x3E: return 'G';
        case 0x3F: return 'C';
        case 0x40: return 'O';
        case 0x41: return 'M';
        case 0x42: return 'N';
        case 0x39: return ' ';
        case 0x2E: return 'c';
        case 0x48: return 'U';
        case 0x50: return 'D';
        case 0x4D: return 'R';
        case 0x4B: return 'L';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x02: return '1';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';
        case 0x20: return 'd';
        case 0x26: return 'l';
        case 0x13: return 'r';
        case 0x16: return 'u';
        case 0x0E: return 'B'; // Backspace
        case 0x1C: return '\n'; // Enter
        case 0x0F: return '\t'; // Tab
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x4A: return '-'; // Keypad -
        case 0x4E: return '+'; // Keypad +
        case 0x37: return '*'; // Keypad *
        case 0x27: return ';';
        case 0x28: return '\'';
        default: return 0;
    }
}

void draw_char(char c, int start_x, int start_y, unsigned short color) {
    if (c < 32 || c > 126) return;
    int font_idx = c - 32;
    unsigned char* bitmap = (unsigned char*)max_font + (font_idx * 8);
    for (int y = 0; y < 8; y++) {
        unsigned char row = bitmap[y];
        for (int x = 0; x < 8; x++) {
            if ((row & (0x80 >> x)) != 0) {
                int px = start_x + x;
                int py = start_y + y;
                if (px >= 0 && px < 1024 && py >= 0 && py < 768) {
                    if (cursor_bg_saved &&
                        px >= cursor_saved_x && px < cursor_saved_x + 12 &&
                        py >= cursor_saved_y && py < cursor_saved_y + 12) {
                        cursor_bg[py - cursor_saved_y][px - cursor_saved_x] = color;
                        if (mouse_arrow[py - cursor_saved_y][px - cursor_saved_x] != 0) {
                            continue;
                        }
                    }
                    gfx_memory[py * 1024 + px] = color;
                }
            }
        }
    }
}
unsigned char bcd_to_binary(unsigned char bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}
unsigned char read_rtc_register(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}
void draw_3d_box(int bx, int by, int bw, int bh, int sunken, unsigned short fill) {
    draw_rect(bx, by, bw, bh, 0x0000);
    if (!sunken) {
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
        draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    } else {
        // Sunken / Pressed 3D Box
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0x39E7);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0x39E7);
        draw_rect(bx + 2, by + 2, bw - 4, 1, 0x7BEF);
        draw_rect(bx + 2, by + 2, 1, bh - 4, 0x7BEF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0xFFFF);
        draw_rect(bx + 3, by + 3, bw - 5, bh - 5, fill);
    }
}

void draw_ui_btn_state(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col, int pressed) {
    draw_rect(bx, by, bw, bh, 0x0000);
    if (!pressed) {
        // Normal raised state
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
        draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);

        int len = (int)strlen(label);
        int tx = bx + (bw - (len * 9)) / 2;
        int ty = by + (bh - 8) / 2;
        if (tx < bx + 2) tx = bx + 2;
        if (ty < by + 1) ty = by + 1;
        print_string((char*)label, tx, ty, text_col);
    } else {
        // Pressed (sunken) state: inverted 3D borders, darkened fill, and (+1, +1) text shift!
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0x39E7);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0x39E7);
        draw_rect(bx + 2, by + 2, bw - 4, 1, 0x7BEF);
        draw_rect(bx + 2, by + 2, 1, bh - 4, 0x7BEF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0xFFFF);

        unsigned short pressed_fill = fill;
        if (fill == 0xCE79 || fill == 0xEF59 || fill == 0xBDD7 || fill == 0xFFFF) {
            pressed_fill = 0x9CD3; // Classic pressed dark gray
        } else if (fill == 0x3DF2) {
            pressed_fill = 0x23EA;
        } else if (fill == 0x24EE) {
            pressed_fill = 0x11EB;
        } else if (fill == 0xF800) {
            pressed_fill = 0x9800;
        } else if (fill == 0xF621) {
            pressed_fill = 0xCE00;
        } else if (fill == 0xC618) {
            pressed_fill = 0x8410;
        } else if (fill == 0x7BEF) {
            pressed_fill = 0x52AA;
        } else if (fill == 0x05E0) {
            pressed_fill = 0x03C0;
        } else if (fill == 0x0DE5) {
            pressed_fill = 0x0A43;
        } else if (fill == 0xF9A6) {
            pressed_fill = 0xD880;
        } else if (fill == 0x8000) {
            pressed_fill = 0x5000;
        }

        draw_rect(bx + 3, by + 3, bw - 5, bh - 5, pressed_fill);

        int len = (int)strlen(label);
        int tx = bx + (bw - (len * 9)) / 2 + 1;
        int ty = by + (bh - 8) / 2 + 1;
        if (tx < bx + 3) tx = bx + 3;
        if (ty < by + 2) ty = by + 2;
        print_string((char*)label, tx, ty, text_col);
    }
}

void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_ui_btn_state(bx, by, bw, bh, label, fill, text_col, 0);
}

void ui_btn_click_effect(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    prev_cursor();
    draw_ui_btn_state(bx, by, bw, bh, label, fill, text_col, 1);
    draw_cursor(pos_x, pos_y);
    play_sound(800);
    sleep(30);
    no_sound();
    
    prev_cursor();
    draw_ui_btn_state(bx, by, bw, bh, label, fill, text_col, 0);
    draw_cursor(pos_x, pos_y);
}

void draw_ui_button(int x, int y, int w, int h, const char* text, unsigned short bg_col, unsigned short text_col, int sunken) {
    draw_rect(x, y, w, h, 0x0000);
    if (!sunken) {
        draw_rect(x + 1, y + 1, w - 2, 1, 0xFFFF);
        draw_rect(x + 1, y + 1, 1, h - 2, 0xFFFF);
        draw_rect(x + w - 2, y + 1, 1, h - 2, 0x7BEF);
        draw_rect(x + 1, y + h - 2, w - 2, 1, 0x7BEF);
        draw_rect(x + 2, y + 2, w - 4, h - 4, bg_col);

        int len = (int)strlen(text);
        int tx = x + (w - (len * 9)) / 2;
        int ty = y + (h - 8) / 2;
        if (tx < x + 2) tx = x + 2;
        if (ty < y + 1) ty = y + 1;
        print_string((char*)text, tx, ty, text_col);
    } else {
        draw_rect(x + 1, y + 1, w - 2, 1, 0x39E7);
        draw_rect(x + 1, y + 1, 1, h - 2, 0x39E7);
        draw_rect(x + 2, y + 2, w - 4, 1, 0x7BEF);
        draw_rect(x + 2, y + 2, 1, h - 4, 0x7BEF);
        draw_rect(x + w - 2, y + 1, 1, h - 2, 0xFFFF);
        draw_rect(x + 1, y + h - 2, w - 2, 1, 0xFFFF);

        unsigned short sunken_col = bg_col;
        if (bg_col == 0xCE79 || bg_col == 0xEF59 || bg_col == 0xBDD7) sunken_col = 0x9CD3;
        else if (bg_col == 0x05E0) sunken_col = 0x03C0;
        else if (bg_col == 0x03EA) sunken_col = 0x0280;

        draw_rect(x + 3, y + 3, w - 5, h - 5, sunken_col);

        int len = (int)strlen(text);
        int tx = x + (w - (len * 9)) / 2 + 1;
        int ty = y + (h - 8) / 2 + 1;
        if (tx < x + 3) tx = x + 3;
        if (ty < y + 2) ty = y + 2;
        print_string((char*)text, tx, ty, text_col);
    }
}

void ui_button_click_effect(int x, int y, int w, int h, const char* text, unsigned short bg_col, unsigned short text_col) {
    prev_cursor();
    draw_ui_button(x, y, w, h, text, bg_col, text_col, 1);
    draw_cursor(pos_x, pos_y);
    play_sound(750);
    sleep(30);
    no_sound();
}

int desktop_handle_click(int mouse_x, int mouse_y) {
    // 1. Check Desktop Icons on the left side
    if (mouse_x >= 10 && mouse_x <= 95) {
        for (int i = 0; i < 8; i++) {
            int iy = 16 + (i * 84);
            if (mouse_y >= iy - 4 && mouse_y <= iy + 74) {
                if (i == 0) maxp_launch_app(MAXP_APP_NOTEPAD);
                else if (i == 1) maxp_launch_app(MAXP_APP_EXPLORER);
                else if (i == 2) maxp_launch_app(MAXP_APP_CALC);
                else if (i == 3) maxp_launch_app(MAXP_APP_SYSINFO);
                else if (i == 4) maxp_launch_app(MAXP_APP_MEM);
                else if (i == 5) maxp_launch_app(MAXP_APP_PONG);
                else if (i == 6) maxp_launch_app(MAXP_APP_INSTALLER);
                else if (i == 7) {
                    maxp_launch_app(MAXP_APP_NOTEPAD);
                    notepad_open_file_by_name("readme.txt");
                    draw_window();
                    draw_cursor(pos_x, pos_y);
                }
                return 1;
            }
        }
    }

    // 2. Check Welcome Card 3D Buttons when card is visible
    int active_app = maxp_get_active_app();
    if (active_app == MAXP_APP_NONE || taskbar_is_app_minimized()) {
        int card_x = 220, card_y = 150;

        // Row 1 (y = card_y + 142 .. card_y + 168)
        if (mouse_y >= card_y + 142 && mouse_y <= card_y + 168) {
            if (mouse_x >= card_x + 30 && mouse_x <= card_x + 165) {
                ui_button_click_effect(card_x + 30, card_y + 142, 135, 26, "Notepad.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_NOTEPAD);
                return 1;
            }
            if (mouse_x >= card_x + 175 && mouse_x <= card_x + 310) {
                ui_button_click_effect(card_x + 175, card_y + 142, 135, 26, "Explorer.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_EXPLORER);
                return 1;
            }
            if (mouse_x >= card_x + 320 && mouse_x <= card_x + 455) {
                ui_button_click_effect(card_x + 320, card_y + 142, 135, 26, "Calc.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_CALC);
                return 1;
            }
            if (mouse_x >= card_x + 465 && mouse_x <= card_x + 610) {
                ui_button_click_effect(card_x + 465, card_y + 142, 145, 26, "SysInfo.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_SYSINFO);
                return 1;
            }
        }

        // Row 2 (y = card_y + 178 .. card_y + 204)
        if (mouse_y >= card_y + 178 && mouse_y <= card_y + 204) {
            if (mouse_x >= card_x + 30 && mouse_x <= card_x + 165) {
                ui_button_click_effect(card_x + 30, card_y + 178, 135, 26, "Mem.maxP", 0x05E0, 0x0000);
                maxp_launch_app(MAXP_APP_MEM);
                return 1;
            }
            if (mouse_x >= card_x + 175 && mouse_x <= card_x + 310) {
                ui_button_click_effect(card_x + 175, card_y + 178, 135, 26, "Pong.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_PONG);
                return 1;
            }
            if (mouse_x >= card_x + 320 && mouse_x <= card_x + 455) {
                ui_button_click_effect(card_x + 320, card_y + 178, 135, 26, "Install.maxP", 0xCE79, 0x0000);
                maxp_launch_app(MAXP_APP_INSTALLER);
                return 1;
            }
            if (mouse_x >= card_x + 465 && mouse_x <= card_x + 610) {
                ui_button_click_effect(card_x + 465, card_y + 178, 145, 26, "Ring 3 Demo", 0x03EA, 0xFFFF);
                ring3_demo_launch();
                return 1;
            }
        }

        // Row 3 (y = card_y + 214 .. card_y + 240)
        if (mouse_y >= card_y + 214 && mouse_y <= card_y + 240) {
            if (mouse_x >= card_x + 30 && mouse_x <= card_x + 610) {
                ui_button_click_effect(card_x + 30, card_y + 214, 580, 26, "Cycle Desktop Theme (1-9)", 0xBDD7, 0x0000);
                theme = (theme % 9) + 1;
                draw_window();
                draw_cursor(pos_x, pos_y);
                return 1;
            }
        }
    }

    return 0;
}

void draw_window() {
    prev_cursor();

    // 1. Draw desktop wallpaper pattern for entire desktop
    static const struct {
        unsigned short c1, c2;
    } theme_wallpapers[10] = {
        { 0x10A2, 0x2124 }, // 0
        { 0x10A2, 0x2124 }, // 1: Classic Steel
        { 0x4002, 0x8085 }, // 2: Crimson
        { 0x2080, 0x4100 }, // 3: Forest
        { 0x10A2, 0x2945 }, // 4: Royal Blue
        { 0x010A, 0x03EF }, // 5: Cyan
        { 0x0102, 0x05E0 }, // 6: Emerald
        { 0x4004, 0xFBEF }, // 7: Rose
        { 0xE62F, 0x8B04 }, // 8: Amber
        { 0x0801, 0x2000 }  // 9: Obsidian
    };
    int t_wp = (theme >= 1 && theme <= 9) ? theme : 1;
    unsigned short wp_c1 = theme_wallpapers[t_wp].c1;
    unsigned short wp_c2 = theme_wallpapers[t_wp].c2;

    for (int y = 0; y < 730; y++) {
        int row_offset = y << 10;
        for (int x = 0; x < 1024; x++) {
            gfx_memory[row_offset + x] = (((x ^ y) & 16) == 0) ? wp_c1 : wp_c2;
        }
    }

    // 2. Desktop icons on the left side
    taskbar_draw_desktop_icons();

    // 3. Desktop center widget if no app is active or if active app is minimized
    int active_app = maxp_get_active_app();
    if (maxp_get_instance_count() == 0 || taskbar_is_app_minimized()) {
        int card_x = 220, card_y = 150, card_w = 640, card_h = 370;
        for (int y = card_y; y < card_y + card_h; y++) {
            for (int x = card_x; x < card_x + card_w; x++) {
                if (y == card_y || y == card_y + card_h - 1 || x == card_x || x == card_x + card_w - 1) {
                    gfx_memory[y * 1024 + x] = 0x0000;
                } else if (y < card_y + 24) {
                    gfx_memory[y * 1024 + x] = 0x11EB;
                } else {
                    gfx_memory[y * 1024 + x] = 0xEF59;
                }
            }
        }
        print_string("Welcome to maxOS MaxRing v3.5 x86_64", card_x + 14, card_y + 6, 0xFFFF);
        print_string("Desktop & Applications Environment", card_x + 25, card_y + 36, 0x11EB);
        print_string("Modular applications use .maxP executable files on maxFS 2.0", card_x + 25, card_y + 58, 0x0000);
        print_string("- Launch applications from Desktop icons on the left side", card_x + 25, card_y + 78, 0x0000);
        print_string("- Or click [maxOS] Start Menu button on bottom-left", card_x + 25, card_y + 96, 0x0000);
        print_string("- Click any button below to launch an application directly:", card_x + 25, card_y + 116, 0x0200);

        // Row 1 3D Buttons
        draw_ui_button(card_x + 30, card_y + 142, 135, 26, "Notepad.maxP", 0xCE79, 0x0000, 0);
        draw_ui_button(card_x + 175, card_y + 142, 135, 26, "Explorer.maxP", 0xCE79, 0x0000, 0);
        draw_ui_button(card_x + 320, card_y + 142, 135, 26, "Calc.maxP", 0xCE79, 0x0000, 0);
        draw_ui_button(card_x + 465, card_y + 142, 145, 26, "SysInfo.maxP", 0xCE79, 0x0000, 0);

        // Row 2 3D Buttons
        draw_ui_button(card_x + 30, card_y + 178, 135, 26, "Mem.maxP", 0x05E0, 0x0000, 0);
        draw_ui_button(card_x + 175, card_y + 178, 135, 26, "Pong.maxP", 0xCE79, 0x0000, 0);
        draw_ui_button(card_x + 320, card_y + 178, 135, 26, "Install.maxP", 0xCE79, 0x0000, 0);
        draw_ui_button(card_x + 465, card_y + 178, 145, 26, "Ring 3 Demo", 0x03EA, 0xFFFF, 0);

        // Row 3 3D Buttons
        draw_ui_button(card_x + 30, card_y + 214, 580, 26, "Change Desktop Theme (Keys 1-9)", 0xBDD7, 0x0000, 0);

        print_string("Security: Ring 3 CPL=3 Protected User Space (All Apps)", card_x + 25, card_y + 254, 0x0200);
        print_string("Hardware: 64-bit IDT Interrupts | PIT 1000Hz (Non-blocking)", card_x + 25, card_y + 274, 0x11EB);
        print_string("Hotkeys: Keys 1-9 switch themes | U: Ring 3 Demo | Esc/c: Close", card_x + 25, card_y + 296, 0x7BEF);
        print_string("maxOS Desktop v3.5 MaxRing x86_64 - by maxTech", card_x + 25, card_y + 326, 0x8085);
    } else {
        // Render active window
        if (get_cpl() == 3) {
            maxp_draw_active_instance();
        } else {
            ring3_app_draw(active_app);
        }
    }

    // 4. Taskbar across the bottom
    taskbar_draw();

    // 5. Cursor
    draw_cursor(pos_x, pos_y);
}

void print_string(char* str, int x, int y, unsigned short color) {
    if (get_cpl() == 3) {
        u_print_string(str, x, y, color);
        return;
    }
    while (*str != 0) {
        draw_char(*str, x, y, color);
        x += 9;
        str++;
    }
}
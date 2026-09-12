#include "verbose_boot.h"
#include "kernel.h"
#include "debug.h"
#include "string.h"
#include "idt.h"
#include "../drivers/video.h"

extern void get_cpu(char* buffer);
extern void draw_rect(int rx, int ry, int rw, int rh, unsigned short color);
extern void print_string(char* str, int x, int y, unsigned short color);
extern void draw_char(char c, int start_x, int start_y, unsigned short color);

#define COLOR_BG          0x0000
#define COLOR_HEADER_BG   0x10E4
#define COLOR_BORDER      0x2965
#define COLOR_TITLE       0xFFFF
#define COLOR_SUBTITLE    0x9CD3
#define COLOR_TIME        0x52AA
#define COLOR_TEXT        0xCE79
#define COLOR_PROGRESS_BG 0x10A4
#define COLOR_PROGRESS_FG 0x05BF

static int verbose_active = 0;
static int verbose_paused = 0;
static int verbose_line_y = 52;
static int current_percent = 0;
static unsigned long ram_mb = 0;
static char cpu_name[64] = "x86_64 Processor";

static int contains_str(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        if (!*n) return 1;
        haystack++;
    }
    return 0;
}

int verbose_boot_is_active(void) {
    return verbose_active;
}

static unsigned short get_tag_color(const char* tag) {
    if (strcmp(tag, "SYSTEM") == 0)  return 0x03EA; // Cyan
    if (strcmp(tag, "MMU") == 0)     return 0x05BF; // Sky Blue
    if (strcmp(tag, "PMM") == 0)     return 0x07E0; // Green
    if (strcmp(tag, "IDT") == 0)     return 0xCE79; // Light Gray
    if (strcmp(tag, "USER") == 0)    return 0x11EB; // Blue
    if (strcmp(tag, "SCHED") == 0)   return 0xFDF3; // Pink/Purple
    if (strcmp(tag, "PCI") == 0)     return 0xFBE0; // Orange
    if (strcmp(tag, "AHCI") == 0)    return 0x07FF; // Cyan
    if (strcmp(tag, "ATA") == 0)     return 0xF621; // Gold
    if (strcmp(tag, "STORAGE") == 0) return 0xF621; // Gold
    if (strcmp(tag, "FS") == 0)      return 0x9CD3; // Light Slate
    if (strcmp(tag, "APPS") == 0)    return 0x07E0; // Emerald
    if (strcmp(tag, "INIT") == 0)    return 0xFFFF; // White
    return 0xCE79;
}

static void draw_badge(int x, int y, int status) {
    const char* label = " INFO ";
    unsigned short col = 0x07FF;
    unsigned short bg_col = 0x0168;

    switch (status) {
        case BOOT_STATUS_OK:
            label = "  OK  ";
            col = 0x07E0;      // Bright green
            bg_col = 0x0200;   // Dark green bg
            break;
        case BOOT_STATUS_INFO:
            label = " INFO ";
            col = 0x07FF;      // Cyan
            bg_col = 0x0168;
            break;
        case BOOT_STATUS_WARN:
            label = " WARN ";
            col = 0xFBE0;      // Yellow/amber
            bg_col = 0x4962;
            break;
        case BOOT_STATUS_FAIL:
            label = " FAIL ";
            col = 0xF800;      // Red
            bg_col = 0x4000;
            break;
        case BOOT_STATUS_SKIP:
            label = " SKIP ";
            col = 0x8410;      // Muted gray
            bg_col = 0x2104;
            break;
        case BOOT_STATUS_DONE:
            label = " DONE ";
            col = 0x07E0;
            bg_col = 0x0200;
            break;
        default:
            return;
    }

    draw_rect(x - 2, y - 2, 76, 12, bg_col);
    print_string((char*)label, x, y, col);
}

static void verbose_boot_scroll(void) {
    const int top = 52;
    const int bottom = 685;
    const int line_h = 15;
    video_scroll(10, top, 1024 - 20, bottom - top + 1, line_h, COLOR_BG);
    verbose_line_y = bottom - line_h + 1;
}

static void verbose_boot_draw_chrome(void) {
    // Clear entire screen to terminal black
    video_clear(COLOR_BG);

    // Top Header Banner
    draw_rect(0, 0, 1024, 44, COLOR_HEADER_BG);
    draw_rect(0, 44, 1024, 1, COLOR_BORDER);

    print_string("maxOS v4.0 EventUpdate (x86_64 Long Mode) - Verbose System Initialization Log", 20, 8, COLOR_TITLE);

    char sub[128];
    strcpy(sub, "CPU: ");
    strcat(sub, cpu_name);
    strcat(sub, " | RAM: ");
    char ram_str[16];
    int_to_str((int)ram_mb, ram_str);
    strcat(sub, ram_str);
    strcat(sub, " MB | Display: ");
    strcat(sub, video_get_mode_name());
    strcat(sub, " | Kernel: ELF64");
    print_string(sub, 20, 24, COLOR_SUBTITLE);

    // Footer divider and hotkeys
    draw_rect(0, 694, 1024, 1, COLOR_BORDER);
    print_string("Hotkeys: [ESC] Graphical Splash / Verbose  |  [SPACE] Pause Log  |  Any key: Launch Desktop Immediately", 20, 736, 0x8410);

    // Progress bar frame
    verbose_boot_progress(current_percent, "Initializing system subsystems...");
}

void verbose_boot_progress(int percent, const char* label) {
    current_percent = percent;
    if (!verbose_active) return;

    int bx = 20;
    int by = 704;
    int bw = 984;
    int bh = 20;

    // Background container
    draw_rect(bx, by, bw, bh, COLOR_PROGRESS_BG);
    draw_rect(bx, by, bw, 1, 0x39E7);
    draw_rect(bx, by + bh - 1, bw, 1, 0x39E7);
    draw_rect(bx, by, 1, bh, 0x39E7);
    draw_rect(bx + bw - 1, by, 1, bh, 0x39E7);

    // Progress fill
    int fill_w = ((bw - 4) * percent) / 100;
    if (fill_w > 0) {
        draw_rect(bx + 2, by + 2, fill_w, bh - 4, COLOR_PROGRESS_FG);
    }

    // Text overlay inside progress bar
    char text[128];
    text[0] = '[';
    text[1] = ' ';
    char p_str[8];
    int_to_str(percent, p_str);
    strcat(text, p_str);
    strcat(text, "% ] ");
    strcat(text, label);
    print_string(text, bx + 15, by + 6, 0xFFFF);
}

void verbose_boot_init(int enable, unsigned int mem_upper) {
    verbose_active = enable;
    verbose_line_y = 52;
    current_percent = 5;
    ram_mb = (mem_upper + 1024) / 1024;
    if (ram_mb == 0) ram_mb = 128;

    char cpubuf[50];
    get_cpu(cpubuf);
    if (cpubuf[0] != '\0') {
        int i = 0;
        while (cpubuf[i] == ' ') i++;
        strncpy(cpu_name, cpubuf + i, sizeof(cpu_name) - 1);
        cpu_name[sizeof(cpu_name) - 1] = '\0';
    }

    if (verbose_active) {
        verbose_boot_draw_chrome();
    }
}

void verbose_boot_check_keys(void) {
    while (inb(0x64) & 1) {
        unsigned char sc = inb(0x60);
        if (sc == 0x2F || sc == 0x01 || sc == 0x0F) { // 'V', ESC, TAB
            verbose_boot_toggle();
            break;
        } else if (sc == 0x39) { // SPACE: Toggle Pause
            verbose_paused = !verbose_paused;
            if (verbose_paused && verbose_active) {
                print_string("[ LOG PAUSED - PRESS SPACE TO RESUME ]", 340, 736, 0xF800);
                while (1) {
                    if (inb(0x64) & 1) {
                        unsigned char unpause_sc = inb(0x60);
                        if (unpause_sc == 0x39 || unpause_sc == 0x01) {
                            verbose_paused = 0;
                            print_string("Hotkeys: [ESC] Graphical Splash / Verbose  |  [SPACE] Pause Log  |  Any key: Launch Desktop Immediately", 20, 736, 0x8410);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void verbose_boot_log(const char* tag, const char* msg) {
    if (!verbose_active) return;

    if (verbose_line_y >= 680) {
        verbose_boot_scroll();
    }

    // 1. Format timestamp: [ 0.042 ]
    unsigned long long ms = system_ticks;
    unsigned int sec = (unsigned int)(ms / 1000);
    unsigned int rem_ms = (unsigned int)(ms % 1000);
    char time_str[16];
    time_str[0] = '[';
    time_str[1] = ' ';
    int_to_str(sec, time_str + 2);
    strcat(time_str, ".");
    if (rem_ms < 100) strcat(time_str, "0");
    if (rem_ms < 10) strcat(time_str, "0");
    char rem_str[8];
    int_to_str(rem_ms, rem_str);
    strcat(time_str, rem_str);
    strcat(time_str, " ]");
    print_string(time_str, 16, verbose_line_y, COLOR_TIME);

    // 2. Format tag: [TAG]
    char tag_str[16];
    tag_str[0] = '[';
    tag_str[1] = '\0';
    strncat(tag_str, tag, 8);
    strcat(tag_str, "]");
    print_string(tag_str, 95, verbose_line_y, get_tag_color(tag));

    // 3. Format message (up to 78 chars to not collide with status badge)
    char msg_truncated[82];
    strncpy(msg_truncated, msg, 78);
    msg_truncated[78] = '\0';
    print_string(msg_truncated, 175, verbose_line_y, COLOR_TEXT);

    // 4. Auto-detect status code
    int status_code = BOOT_STATUS_INFO;
    if (contains_str(msg, "Error") || contains_str(msg, "ERR") || contains_str(msg, "Fault") || contains_str(msg, "Fail")) {
        status_code = BOOT_STATUS_FAIL;
    } else if (contains_str(msg, "Not ") || contains_str(msg, "No ") || contains_str(msg, "None") || contains_str(msg, "SKIP")) {
        status_code = BOOT_STATUS_SKIP;
    } else if (contains_str(msg, "Warn")) {
        status_code = BOOT_STATUS_WARN;
    } else if (contains_str(msg, "Initialized") || contains_str(msg, "Active") || contains_str(msg, "Detected") ||
               contains_str(msg, "completed") || contains_str(msg, "OK") || contains_str(msg, "loaded") ||
               contains_str(msg, "ready") || contains_str(msg, "active")) {
        status_code = BOOT_STATUS_OK;
    }

    draw_badge(925, verbose_line_y, status_code);

    verbose_line_y += 15;
    verbose_boot_check_keys();
}

void verbose_boot_step(const char* tag, const char* msg, int status_code, int percent) {
    if (percent >= 0 && percent <= 100) {
        verbose_boot_progress(percent, msg);
    }

    if (!verbose_active) return;

    if (verbose_line_y >= 680) {
        verbose_boot_scroll();
    }

    unsigned long long ms = system_ticks;
    unsigned int sec = (unsigned int)(ms / 1000);
    unsigned int rem_ms = (unsigned int)(ms % 1000);
    char time_str[16];
    time_str[0] = '[';
    time_str[1] = ' ';
    int_to_str(sec, time_str + 2);
    strcat(time_str, ".");
    if (rem_ms < 100) strcat(time_str, "0");
    if (rem_ms < 10) strcat(time_str, "0");
    char rem_str[8];
    int_to_str(rem_ms, rem_str);
    strcat(time_str, rem_str);
    strcat(time_str, " ]");
    print_string(time_str, 16, verbose_line_y, COLOR_TIME);

    char tag_str[16];
    tag_str[0] = '[';
    tag_str[1] = '\0';
    strncat(tag_str, tag, 8);
    strcat(tag_str, "]");
    print_string(tag_str, 95, verbose_line_y, get_tag_color(tag));

    char msg_truncated[82];
    strncpy(msg_truncated, msg, 78);
    msg_truncated[78] = '\0';
    print_string(msg_truncated, 175, verbose_line_y, 0xFFFF);

    draw_badge(925, verbose_line_y, status_code);

    verbose_line_y += 15;
    verbose_boot_check_keys();
}

static void verbose_boot_replay_history(void) {
    verbose_boot_draw_chrome();
    verbose_line_y = 52;

    int total = debug_history_total;
    if (total > DEBUG_HISTORY_COUNT) total = DEBUG_HISTORY_COUNT;
    int start = (debug_history_head - total + DEBUG_HISTORY_COUNT) % DEBUG_HISTORY_COUNT;

    for (int i = 0; i < total; i++) {
        int idx = (start + i) % DEBUG_HISTORY_COUNT;
        struct DebugLogEntry* e = &debug_history[idx];
        if (e->msg[0] != '\0') {
            verbose_boot_log(e->tag, e->msg);
        }
    }
}

void verbose_boot_set_active(int active) {
    if (verbose_active == active) return;
    verbose_active = active;
    if (verbose_active) {
        verbose_boot_replay_history();
    }
}

void verbose_boot_toggle(void) {
    verbose_boot_set_active(!verbose_active);
}

void verbose_boot_finish(void) {
    if (!verbose_active) return;

    verbose_boot_step("INIT", "All subsystems successfully initialized. System operational.", BOOT_STATUS_DONE, 100);

    print_string("Desktop environment ready. Launching in 2 seconds... (Press any key to enter)", 180, verbose_line_y + 8, 0x07E0);

    unsigned long long end = system_ticks + 1800;
    while (system_ticks < end) {
        if (inb(0x64) & 1) {
            inb(0x60); // Consume keypress
            break;
        }
        sleep(20);
    }
}

#include "sysinfo.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"
#include "kernel.h"
#include "user.h"
#include "debug.h"
#include "task.h"

void get_cpu(char* buffer);

int sysinfo_open = 0;
static sysinfo_state_t primary_sysinfo_state;

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    int len = 0;
    while (label[len] != '\0') len++;
    int tx = bx + (bw - (len * 9)) / 2;
    int ty = by + (bh - 8) / 2;
    if (tx < bx + 2) tx = bx + 2;
    if (ty < by + 1) ty = by + 1;
    print_string((char*)label, tx, ty, text_col);
}

void sysinfo_instance_init(sysinfo_state_t* s) {
    s->dummy = 0;
}

void sysinfo_instance_draw(sysinfo_state_t* s, int sx, int sy, int sw, int sh) {
    (void)s;
    prev_cursor();

    // Window frame
    draw_rect(sx, sy, sw, sh, 0x0000);
    draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x0DE5);
    print_string("maxOS System Hardware & Kernel Information (sysinfo.bin)", sx + 8, sy + 6, 0x0000);

    // [X] Close button
    draw_ui_btn(sx + sw - 22, sy + 3, 18, 16, "X", 0xF800, 0xFFFF);

    // Main Card
    draw_rect(sx + 10, sy + 30, sw - 20, sh - 70, 0xFFFF);
    draw_rect(sx + 10, sy + 30, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    // Section 1: OS Banner
    draw_rect(sx + 15, sy + 38, sw - 30, 32, 0x0DE5);
    print_string("maxOS RedCycle v3.2 (x86_64 Long Mode)", sx + 25, sy + 44, 0x0000);
    print_string("Kernel Architecture: 64-bit AMD64 / Intel 64", sx + 25, sy + 56, 0x0000);

    int row_y = sy + 82;

    // Section 2: Processor Info
    print_string("[ Processor / CPU ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    char cpu_model[49];
    get_cpu(cpu_model);
    print_string("Model: ", sx + 25, row_y, 0x0000);
    print_string(cpu_model, sx + 80, row_y, 0x24EE);
    row_y += 15;
    print_string("Mode:  64-bit Long Mode | PML4 Paging | System V AMD64 ABI", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Interrupts: 64-bit IDT Active | 8259 PIC Remapped | PIT 1000Hz Timer", sx + 25, row_y, 0x03EA);
    row_y += 15;
    long current_cpl = get_cpl();
    if (current_cpl == 3) {
        print_string("Privilege: Ring 3 User Mode (CPL = 3) [ALL APPS PROTECTED]", sx + 25, row_y, 0x03EA);
    } else {
        print_string("Privilege: Ring 0 Kernel Mode (User Mode Ready)", sx + 25, row_y, 0x03EA);
    }
    row_y += 24;

    // Section 3: Video & Memory
    print_string("[ Display & Memory ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    print_string("Graphics: VESA VBE 1024x768 @ 16-bit HighColor (RGB 565)", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Linear Framebuffer Address: 0xFD000000 (Identity Mapped)", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Kernel Load Address:        0x00100000 (1 MB Physical RAM)", sx + 25, row_y, 0x0000);
    row_y += 24;

    // Section 4: Storage & ATA Driver Diagnostics
    print_string("[ Storage & ATA Driver Diagnostics ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    print_string("Storage Device: ATA Primary Master (IDE PIO)", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Drive Model: ", sx + 25, row_y, 0x0000);
    print_string(ata_primary_master.present ? ata_primary_master.model : "Not Detected", sx + 135, row_y, 0x24EE);
    row_y += 15;

    char numbuf[32];
    print_string("Capacity: ", sx + 25, row_y, 0x0000);
    int_str((int)ata_primary_master.size_mb, numbuf);
    print_string(numbuf, sx + 115, row_y, 0x03EA);
    print_string(" MB (", sx + 155, row_y, 0x0000);
    int_str((int)ata_primary_master.total_sectors, numbuf);
    print_string(numbuf, sx + 200, row_y, 0x03EA);
    print_string(" sectors) | maxFS 2.0 Inode FS", sx + 280, row_y, 0x0000);
    row_y += 15;

    const struct ATADebugStats* stats = ata_get_debug_stats();
    print_string("IO Counters: Reads=", sx + 25, row_y, 0x0000);
    int_str((int)stats->reads_count, numbuf);
    print_string(numbuf, sx + 195, row_y, 0x0000);
    print_string(" | Writes=", sx + 245, row_y, 0x0000);
    int_str((int)stats->writes_count, numbuf);
    print_string(numbuf, sx + 325, row_y, 0x0000);
    print_string(" | Errors=", sx + 375, row_y, 0x0000);
    int_str((int)stats->errors_count, numbuf);
    print_string(numbuf, sx + 450, row_y, (stats->errors_count == 0) ? 0x05E0 : 0xF800);

    // Section 5: Preemptive Multitasking
    row_y += 24;
    print_string("[ Preemptive Multitasking & Processes ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    print_string("Scheduler: Round-Robin 20ms Time Slices | 15 CPU Registers Saved | Multi-Instance", sx + 25, row_y, 0x0000);
    row_y += 15;
    int tcount = task_get_count();
    print_string("Active Processes: ", sx + 25, row_y, 0x0000);
    int_str(tcount, numbuf);
    print_string(numbuf, sx + 180, row_y, 0x05E0);
    print_string(" running simultaneously", sx + 205, row_y, 0x0000);

    // Bottom Action Buttons
    draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0x03EA, 0xFFFF);

    draw_cursor(pos_x, pos_y);
}

int sysinfo_instance_click(sysinfo_state_t* s, int sx, int sy, int sw, int sh, int mouse_x, int mouse_y) {
    (void)s;
    // [X] Close button (Titlebar)
    if (mouse_x >= sx + sw - 26 && mouse_x <= sx + sw && mouse_y >= sy && mouse_y <= sy + 24) {
        return -1;
    }

    // Refresh button
    if (mouse_x >= sx + 20 && mouse_x <= sx + 120 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        draw_window();
        play_sound(750); sleep(30); no_sound();
        return 1;
    }

    // Ring 3 Demo button
    if (mouse_x >= sx + 130 && mouse_x <= sx + 310 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        ring3_demo_launch();
        return 1;
    }

    if (mouse_x >= sx && mouse_x <= sx + sw && mouse_y >= sy && mouse_y <= sy + sh) {
        return 1;
    }

    return 0;
}

int sysinfo_instance_key(sysinfo_state_t* s, char ascii_char, unsigned char scan_code) {
    (void)s;
    if (ascii_char == 'u' || ascii_char == 'U') {
        ring3_demo_launch();
        return 1;
    }

    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01 || scan_code == 0x3C) {
        return -1;
    }

    if (ascii_char == 'r' || ascii_char == 'R') {
        draw_window();
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void sysinfo_init(void) {
    sysinfo_open = 0;
    sysinfo_instance_init(&primary_sysinfo_state);
}

void sysinfo_open_window(void) {
    sysinfo_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_SYSINFO, "SysInfo", 0);
}

void sysinfo_close_window(void) {
    sysinfo_open = 0;
    maxp_close_app(MAXP_APP_SYSINFO);
}

void sysinfo_draw(void) {
    sysinfo_instance_draw(&primary_sysinfo_state, win_x + 20, win_y + 35, win_w - 40, win_h - 45);
}

int sysinfo_handle_click(int mouse_x, int mouse_y) {
    int res = sysinfo_instance_click(&primary_sysinfo_state, win_x + 20, win_y + 35, win_w - 40, win_h - 45, mouse_x, mouse_y);
    if (res == -1) {
        sysinfo_close_window();
        return 1;
    }
    return res;
}

int sysinfo_handle_key(char ascii_char, unsigned char scan_code) {
    int res = sysinfo_instance_key(&primary_sysinfo_state, ascii_char, scan_code);
    if (res == -1) {
        sysinfo_close_window();
        return 1;
    }
    return res;
}

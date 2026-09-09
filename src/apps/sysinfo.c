#include "sysinfo.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"

#include "kernel.h"

void get_cpu(char* buffer);

int sysinfo_open = 0;

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 8, by + 5, text_col);
}

void sysinfo_init(void) {
    sysinfo_open = 0;
}

void sysinfo_open_window(void) {
    sysinfo_open = 1;
    drag = 1;
    maxp_set_active_app(MAXP_APP_SYSINFO);
    draw_window();
}

void sysinfo_close_window(void) {
    sysinfo_open = 0;
    drag = 0;
    maxp_set_active_app(MAXP_APP_NONE);
    draw_window();
    draw_cursor(pos_x, pos_y);
}

void sysinfo_draw(void) {
    prev_cursor();

    int sx = win_x + 20;
    int sy = win_y + 35;
    int sw = win_w - 40;
    int sh = win_h - 45;

    // Window frame
    draw_rect(sx, sy, sw, sh, 0x0000);
    draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    draw_rect(sx + 2, sy + 2, sw - 4, 22, 0x0320); // Dark teal title
    print_string("System Information - [sysinfo.maxP]", sx + 8, sy + 6, 0xFFFF);

    // [Close] button on top-right
    draw_ui_btn(sx + sw - 56, sy + 3, 50, 18, "Close", 0xF9A6, 0x0000);

    // Main Card
    draw_rect(sx + 10, sy + 30, sw - 20, sh - 70, 0xFFFF);
    draw_rect(sx + 10, sy + 30, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    // Section 1: OS Banner
    draw_rect(sx + 15, sy + 38, sw - 30, 32, 0x0DE5);
    print_string("maxOS RedCycle v3.0 (x86_64 Long Mode)", sx + 25, sy + 44, 0x0000);
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
    print_string("Mode:  64-bit Long Mode | PML4 4-Level Paging (2MB Huge Pages)", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Features: PAE Enabled | SSE/SSE2 Capable | System V AMD64 ABI", sx + 25, row_y, 0x0000);
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

    // Section 4: Storage & Filesystem
    print_string("[ Storage & Filesystem ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    print_string("Filesystem: maxFS 2.0 Inode-Based Persistent Filesystem", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Storage Device: ATA Primary Master (IDE PIO Mode)", sx + 25, row_y, 0x0000);
    row_y += 15;
    print_string("Program Executable Format: .maxP (maxOS Program)", sx + 25, row_y, 0x0000);

    // Bottom action buttons
    draw_ui_btn(sx + 20, sy + sh - 34, 80, 22, "Refresh", 0xC618, 0x0000);
    draw_ui_btn(sx + sw - 100, sy + sh - 34, 80, 22, "Close (c)", 0xF9A6, 0x0000);

    draw_cursor(pos_x, pos_y);
}

int sysinfo_handle_click(int mouse_x, int mouse_y) {
    if (!sysinfo_open) return 0;

    int sx = win_x + 20;
    int sy = win_y + 35;
    int sw = win_w - 40;
    int sh = win_h - 45;

    // Titlebar close button
    if (mouse_x >= sx + sw - 56 && mouse_x <= sx + sw - 6 && mouse_y >= sy + 3 && mouse_y <= sy + 21) {
        sysinfo_close_window();
        return 1;
    }

    // Bottom close button
    if (mouse_x >= sx + sw - 100 && mouse_x <= sx + sw - 20 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        sysinfo_close_window();
        return 1;
    }

    // Bottom refresh button
    if (mouse_x >= sx + 20 && mouse_x <= sx + 100 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        sysinfo_draw();
        play_sound(700); sleep(30); no_sound();
        return 1;
    }

    // Click inside window
    if (mouse_x >= sx && mouse_x <= sx + sw && mouse_y >= sy && mouse_y <= sy + sh) {
        return 1;
    }

    return 0;
}

int sysinfo_handle_key(char ascii_char, unsigned char scan_code) {
    if (!sysinfo_open) return 0;

    // 'c', 'q', or Esc (0x01) or F2 (0x3C)
    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01 || scan_code == 0x3C) {
        sysinfo_close_window();
        return 1;
    }

    // 'r' or 'R' refreshes
    if (ascii_char == 'r' || ascii_char == 'R') {
        sysinfo_draw();
        return 1;
    }

    return 0;
}

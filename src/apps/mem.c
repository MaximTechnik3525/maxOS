#include "mem.h"
#include "kernel.h"
#include "maxp.h"
#include "maxfs.h"
#include "user.h"
#include "debug.h"

int mem_open = 0;
static int optimize_flash = 0;

/* Button helper with 3D bevel */
static void draw_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
    draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 8, by + 4, text_col);
}

/* Format KB to string "X.YY MB" or "X MB" */
static void format_kb_to_mb(unsigned int kb, char* out) {
    unsigned int mb_whole = kb / 1024;
    unsigned int mb_frac = ((kb % 1024) * 100) / 1024;

    char whole_str[16];
    char frac_str[16];
    int_str((int)mb_whole, whole_str);
    int_str((int)mb_frac, frac_str);

    int idx = 0;
    for (int i = 0; whole_str[i] != '\0'; i++) out[idx++] = whole_str[i];
    out[idx++] = '.';
    if (mb_frac < 10) out[idx++] = '0';
    for (int i = 0; frac_str[i] != '\0'; i++) out[idx++] = frac_str[i];
    out[idx++] = ' ';
    out[idx++] = 'M';
    out[idx++] = 'B';
    out[idx++] = '\0';
}

void mem_init(void) {
    mem_open = 0;
    optimize_flash = 0;
}

void mem_open_window(void) {
    mem_open = 1;
    optimize_flash = 0;
    drag = 1;
    maxp_set_active_app(MAXP_APP_MEM);
    draw_window();
}

void mem_close_window(void) {
    mem_open = 0;
    drag = 0;
    optimize_flash = 0;
    maxp_set_active_app(MAXP_APP_NONE);
    draw_window();
    draw_cursor(pos_x, pos_y);
}

void mem_draw(void) {
    prev_cursor();

    struct SystemMemInfo mi;
    get_system_mem_info(&mi);

    int sx = win_x + 20;
    int sy = win_y + 35;
    int sw = win_w - 40;
    int sh = win_h - 45;

    // Window frame (Classic 3D border)
    draw_rect(sx, sy, sw, sh, 0x0000);
    draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x041F); // Royal Cyan/Navy
    print_string("maxOS RAM & Memory Monitor (mem.maxP)", sx + 8, sy + 6, 0xFFFF);

    // Close button on titlebar
    draw_btn(sx + sw - 56, sy + 3, 50, 18, "Close", 0xF9A6, 0x0000);

    // Main Card background
    draw_rect(sx + 10, sy + 28, sw - 20, sh - 68, 0xFFFF);
    draw_rect(sx + 10, sy + 28, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    // Header banner
    draw_rect(sx + 14, sy + 34, sw - 28, 28, 0x0DE5);
    print_string("Physical RAM Consumption & Subsystem Allocation", sx + 22, sy + 40, 0x0000);
    print_string("Paging: PML4 4-Level | Mode: Ring 3 Protected | Framebuffer: RGB565", sx + 22, sy + 52, 0x0000);

    // Section 1: Graphical RAM Gauge / Progress Bar
    int bar_x = sx + 20;
    int bar_y = sy + 70;
    int bar_w = sw - 40;
    int bar_h = 24;

    // Progress Bar Inset Border
    draw_rect(bar_x, bar_y, bar_w, bar_h, 0x0000);
    draw_rect(bar_x + 1, bar_y + 1, bar_w - 2, bar_h - 2, 0x2124); // Dark background

    // Compute fill width
    int fill_w = 0;
    if (mi.total_kb > 0) {
        fill_w = (int)(((unsigned long long)mi.used_kb * (bar_w - 4)) / mi.total_kb);
    }
    if (fill_w < 4) fill_w = 4;
    if (fill_w > bar_w - 4) fill_w = bar_w - 4;

    // Draw segmented / solid progress fill
    unsigned short fill_color = optimize_flash ? 0x07E0 : 0x24EE; // Bright cyan or green
    draw_rect(bar_x + 2, bar_y + 2, fill_w, bar_h - 4, fill_color);

    // Progress percentage & values text
    char num_buf[32];
    char used_mb[32];
    char total_mb[32];
    format_kb_to_mb(mi.used_kb, used_mb);
    format_kb_to_mb(mi.total_kb, total_mb);

    print_string("RAM Usage: ", bar_x + 6, bar_y + bar_h + 8, 0x0000);
    int_str((int)mi.usage_percent, num_buf);
    print_string(num_buf, bar_x + 95, bar_y + bar_h + 8, 0x0200);
    print_string("% (Used: ", bar_x + 115, bar_y + bar_h + 8, 0x0000);
    print_string(used_mb, bar_x + 185, bar_y + bar_h + 8, 0x24EE);
    print_string(" / Total: ", bar_x + 265, bar_y + bar_h + 8, 0x0000);
    print_string(total_mb, bar_x + 355, bar_y + bar_h + 8, 0x03EA);
    print_string(")", bar_x + 435, bar_y + bar_h + 8, 0x0000);

    // Section 2: Detailed Statistics Grid
    int col1_x = sx + 20;
    int col2_x = sx + (sw / 2) + 10;
    int stats_y = bar_y + bar_h + 30;

    // Column 1 Box: Memory Statistics
    draw_rect(col1_x, stats_y, (sw / 2) - 20, 155, 0xF7BE);
    draw_rect(col1_x, stats_y, (sw / 2) - 20, 18, 0xBDD7);
    print_string("System RAM Summary", col1_x + 6, stats_y + 4, 0x0000);

    int row_y = stats_y + 24;
    print_string("Total RAM: ", col1_x + 10, row_y, 0x0000);
    print_string(total_mb, col1_x + 110, row_y, 0x11EB);
    row_y += 18;

    print_string("Used RAM:  ", col1_x + 10, row_y, 0x0000);
    print_string(used_mb, col1_x + 110, row_y, 0x8085);
    row_y += 18;

    char free_mb[32];
    format_kb_to_mb(mi.free_kb, free_mb);
    print_string("Free RAM:  ", col1_x + 10, row_y, 0x0000);
    print_string(free_mb, col1_x + 110, row_y, 0x0200);
    row_y += 18;

    print_string("Lower Mem: ", col1_x + 10, row_y, 0x0000);
    int_str((int)mi.lower_kb, num_buf);
    print_string(num_buf, col1_x + 110, row_y, 0x0000);
    print_string(" KB (0-640K)", col1_x + 145, row_y, 0x7BEF);
    row_y += 18;

    print_string("Upper Mem: ", col1_x + 10, row_y, 0x0000);
    int_str((int)mi.upper_kb, num_buf);
    print_string(num_buf, col1_x + 110, row_y, 0x0000);
    print_string(" KB (1MB+)", col1_x + 165, row_y, 0x7BEF);
    row_y += 18;

    if (optimize_flash) {
        print_string("Status:    Optimized (Flushed)", col1_x + 10, row_y, 0x07E0);
    } else {
        print_string("Status:    Active / Protected", col1_x + 10, row_y, 0x03EA);
    }

    // Column 2 Box: Subsystem Memory Breakdown
    draw_rect(col2_x, stats_y, (sw / 2) - 30, 155, 0xF7BE);
    draw_rect(col2_x, stats_y, (sw / 2) - 30, 18, 0xBDD7);
    print_string("Subsystem RAM Breakdown", col2_x + 6, stats_y + 4, 0x0000);

    row_y = stats_y + 24;
    print_string("Kernel Binary: ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.kernel_kb, num_buf);
    print_string(num_buf, col2_x + 150, row_y, 0x0000);
    print_string(" KB", col2_x + 185, row_y, 0x0000);
    row_y += 18;

    print_string("VESA VBE Framebuffer: ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.vram_kb, num_buf);
    print_string(num_buf, col2_x + 195, row_y, 0x0000);
    print_string(" KB", col2_x + 235, row_y, 0x0000);
    row_y += 18;

    print_string("Paging Tables: ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.paging_kb, num_buf);
    print_string(num_buf, col2_x + 150, row_y, 0x0000);
    print_string(" KB (PML4/PD)", col2_x + 175, row_y, 0x7BEF);
    row_y += 18;

    print_string("Kernel & User Stacks: ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.stacks_kb, num_buf);
    print_string(num_buf, col2_x + 195, row_y, 0x0000);
    print_string(" KB", col2_x + 220, row_y, 0x0000);
    row_y += 18;

    print_string("maxFS Ramdisk Cache:  ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.ramdisk_kb, num_buf);
    print_string(num_buf, col2_x + 195, row_y, 0x0000);
    print_string(" KB", col2_x + 220, row_y, 0x0000);
    row_y += 18;

    print_string("Working Heap / Apps:  ", col2_x + 10, row_y, 0x0000);
    int_str((int)mi.apps_dynamic_kb, num_buf);
    print_string(num_buf, col2_x + 195, row_y, 0x0000);
    print_string(" KB", col2_x + 235, row_y, 0x0000);

    // Section 3: Bottom Action Buttons
    draw_btn(sx + 20, sy + sh - 34, 110, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_btn(sx + 140, sy + sh - 34, 145, 22, "Optimize RAM (o)", 0x03EA, 0xFFFF);
    draw_btn(sx + sw - 100, sy + sh - 34, 80, 22, "Close (c)", 0xF9A6, 0x0000);

    if (optimize_flash) {
        print_string("[*] Cache Flushed!", sx + 300, sy + sh - 28, 0x03EA);
        optimize_flash = 0;
    }

    draw_cursor(pos_x, pos_y);
}

int mem_handle_click(int mouse_x, int mouse_y) {
    if (!mem_open) return 0;

    int sx = win_x + 20;
    int sy = win_y + 35;
    int sw = win_w - 40;
    int sh = win_h - 45;

    // Titlebar close button
    if (mouse_x >= sx + sw - 56 && mouse_x <= sx + sw - 6 && mouse_y >= sy + 3 && mouse_y <= sy + 21) {
        mem_close_window();
        return 1;
    }

    // Bottom close button
    if (mouse_x >= sx + sw - 100 && mouse_x <= sx + sw - 20 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        mem_close_window();
        return 1;
    }

    // Bottom refresh button
    if (mouse_x >= sx + 20 && mouse_x <= sx + 130 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        mem_draw();
        play_sound(750); sleep(30); no_sound();
        return 1;
    }

    // Bottom optimize RAM button
    if (mouse_x >= sx + 140 && mouse_x <= sx + 290 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        optimize_flash = 1;
        play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
        mem_draw();
        return 1;
    }

    // Click inside window swallows event
    if (mouse_x >= sx && mouse_x <= sx + sw && mouse_y >= sy && mouse_y <= sy + sh) {
        return 1;
    }

    return 0;
}

int mem_handle_key(char ascii_char, unsigned char scan_code) {
    if (!mem_open) return 0;

    // 'c', 'q', or Esc (0x01) closes
    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01) {
        mem_close_window();
        return 1;
    }

    // 'r' or 'R' refreshes
    if (ascii_char == 'r' || ascii_char == 'R') {
        mem_draw();
        play_sound(750); sleep(20); no_sound();
        return 1;
    }

    // 'o' or 'O' optimizes
    if (ascii_char == 'o' || ascii_char == 'O') {
        optimize_flash = 1;
        play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
        mem_draw();
        return 1;
    }

    return 0;
}

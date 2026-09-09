#include "mem.h"
#include "kernel.h"
#include "maxp.h"
#include "maxfs.h"
#include "user.h"
#include "task.h"
#include "debug.h"

int mem_open = 0;
static int optimize_flash = 0;
static int mem_view_mode = 0; // 0 = RAM Monitor, 1 = Task Manager
static int selected_task_idx = 0;

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
    mem_view_mode = 0;
    selected_task_idx = 0;
}

void mem_open_window(void) {
    mem_open = 1;
    optimize_flash = 0;
    drag = 1;
    selected_task_idx = 0;
    maxp_set_active_app(MAXP_APP_MEM);
    draw_window();
}

void mem_close_window(void) {
    optimize_flash = 0;
    maxp_close_app(MAXP_APP_MEM);
}

static void draw_task_manager(int sx, int sy, int sw, int sh) {
    task_info_t tlist[MAX_TASKS];
    int tcount = task_get_list(tlist, MAX_TASKS);

    // Header banner
    draw_rect(sx + 14, sy + 34, sw - 28, 28, 0x03EA);
    print_string("Preemptive Task Manager (Round-Robin 20ms Scheduler)", sx + 22, sy + 40, 0xFFFF);
    print_string("All processes preemptively scheduled via PIT 1000Hz timer interrupt", sx + 22, sy + 52, 0xFFFF);

    // Table Container Box
    int table_x = sx + 20;
    int table_y = sy + 68;
    int table_w = sw - 40;
    int table_h = sh - 110;

    draw_rect(table_x, table_y, table_w, table_h, 0x0000);
    draw_rect(table_x + 1, table_y + 1, table_w - 2, table_h - 2, 0xFFFF);

    // Table Header Bar
    draw_rect(table_x + 1, table_y + 1, table_w - 2, 22, 0xCE79);
    print_string("PID", table_x + 8, table_y + 6, 0x0000);
    print_string("PROCESS NAME", table_x + 48, table_y + 6, 0x0000);
    print_string("PRIVILEGE", table_x + 190, table_y + 6, 0x0000);
    print_string("STATE", table_x + 345, table_y + 6, 0x0000);
    print_string("CPU TIME", table_x + 440, table_y + 6, 0x0000);
    print_string("STATUS", table_x + 550, table_y + 6, 0x0000);

    if (selected_task_idx >= tcount) selected_task_idx = tcount - 1;
    if (selected_task_idx < 0) selected_task_idx = 0;

    char num_buf[16];
    for (int i = 0; i < tcount && i < 10; i++) {
        int ry = table_y + 24 + (i * 20);
        int is_sel = (i == selected_task_idx);

        unsigned short row_bg = is_sel ? 0x041F : ((i % 2 == 0) ? 0xFFFF : 0xEF59);
        unsigned short text_col = is_sel ? 0xFFFF : 0x0000;
        draw_rect(table_x + 1, ry, table_w - 2, 19, row_bg);

        // PID
        int_str(tlist[i].pid, num_buf);
        print_string(num_buf, table_x + 12, ry + 4, text_col);

        // Name
        print_string(tlist[i].name, table_x + 48, ry + 4, text_col);

        // Privilege
        if (tlist[i].is_user) {
            print_string("Ring 3 (User)", table_x + 190, ry + 4, is_sel ? 0x07E0 : 0x03EA);
        } else {
            print_string("Ring 0 (Kernel)", table_x + 190, ry + 4, is_sel ? 0xCE79 : 0x11EB);
        }

        // State
        const char* st = "UNKNOWN";
        if (tlist[i].state == TASK_READY) st = "READY";
        else if (tlist[i].state == TASK_RUNNING) st = "RUNNING";
        else if (tlist[i].state == TASK_SLEEPING) st = "SLEEPING";
        else if (tlist[i].state == TASK_DEAD) st = "DEAD";
        print_string((char*)st, table_x + 345, ry + 4, text_col);

        // CPU ticks
        int_str((int)tlist[i].total_ticks, num_buf);
        print_string(num_buf, table_x + 440, ry + 4, text_col);
        print_string(" ms", table_x + 485, ry + 4, text_col);

        // Status
        if (tlist[i].state == TASK_RUNNING) {
            print_string("Active", table_x + 550, ry + 4, 0x07E0);
        } else if (tlist[i].state == TASK_READY) {
            print_string("Queued", table_x + 550, ry + 4, text_col);
        } else if (tlist[i].state == TASK_SLEEPING) {
            print_string("Idle", table_x + 550, ry + 4, 0x8085);
        } else {
            print_string("Exited", table_x + 550, ry + 4, 0xF800);
        }
    }

    // Bottom Action Buttons in Task Manager mode:
    draw_btn(sx + 20, sy + sh - 34, 140, 22, "Memory View (m)", 0x24EE, 0xFFFF);
    draw_btn(sx + 170, sy + sh - 34, 110, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_btn(sx + 290, sy + sh - 34, 120, 22, "Kill Task (k)", 0xF800, 0xFFFF);
    draw_btn(sx + sw - 100, sy + sh - 34, 80, 22, "Close (c)", 0xF9A6, 0x0000);
}

void mem_draw(void) {
    prev_cursor();

    int sx = win_x + 20;
    int sy = win_y + 35;
    int sw = win_w - 40;
    int sh = win_h - 45;

    // Window frame (Classic 3D border)
    draw_rect(sx, sy, sw, sh, 0x0000);
    draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x041F); // Royal Cyan/Navy
    print_string("maxOS Memory & Task Manager (mem.maxP)", sx + 8, sy + 6, 0xFFFF);

    // Close button on titlebar
    draw_btn(sx + sw - 56, sy + 3, 50, 18, "Close", 0xF9A6, 0x0000);

    // Main Card background
    draw_rect(sx + 10, sy + 28, sw - 20, sh - 68, 0xFFFF);
    draw_rect(sx + 10, sy + 28, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    if (mem_view_mode == 1) {
        draw_task_manager(sx, sy, sw, sh);
        draw_cursor(pos_x, pos_y);
        return;
    }

    struct SystemMemInfo mi;
    get_system_mem_info(&mi);

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
    draw_rect(bar_x + 1, bar_y + 1, bar_w - 2, bar_h - 2, 0x2124);

    int fill_w = 0;
    if (mi.total_kb > 0) {
        fill_w = (int)(((unsigned long long)mi.used_kb * (bar_w - 4)) / mi.total_kb);
    }
    if (fill_w < 4 && mi.used_kb > 0) fill_w = 4;
    if (fill_w > bar_w - 4) fill_w = bar_w - 4;

    unsigned short bar_fill_col = 0x05E0;
    if (mi.usage_percent > 80) bar_fill_col = 0xF800;
    else if (mi.usage_percent > 50) bar_fill_col = 0xFD20;

    if (fill_w > 0) {
        draw_rect(bar_x + 2, bar_y + 2, fill_w, bar_h - 4, bar_fill_col);
    }

    // RAM summary string below gauge
    char total_mb[32];
    char used_mb[32];
    format_kb_to_mb(mi.total_kb, total_mb);
    format_kb_to_mb(mi.used_kb, used_mb);

    char pct_str[16];
    int_str((int)mi.usage_percent, pct_str);

    print_string("RAM Usage: ", bar_x, bar_y + 30, 0x0000);
    print_string(pct_str, bar_x + 95, bar_y + 30, 0x0000);
    print_string("%  (Used: ", bar_x + 125, bar_y + 30, 0x0000);
    print_string(used_mb, bar_x + 195, bar_y + 30, 0x03EA);
    print_string("/ Total: ", bar_x + 280, bar_y + 30, 0x0000);
    print_string(total_mb, bar_x + 355, bar_y + 30, 0x0200);
    print_string(")", bar_x + 440, bar_y + 30, 0x0000);

    // Section 2: Detailed Breakdown Panels
    int stats_y = bar_y + 54;
    int col1_x = sx + 20;
    int col2_x = sx + (sw / 2) + 10;

    // Column 1 Box: System RAM Summary
    draw_rect(col1_x, stats_y, (sw / 2) - 30, 155, 0xF7BE);
    draw_rect(col1_x, stats_y, (sw / 2) - 30, 18, 0xBDD7);
    print_string("System RAM Summary", col1_x + 6, stats_y + 4, 0x0000);

    int row_y = stats_y + 24;
    char num_buf[32];

    print_string("Total RAM: ", col1_x + 10, row_y, 0x0000);
    print_string(total_mb, col1_x + 110, row_y, 0x0000);
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

    // Bottom Action Buttons in Memory mode:
    draw_btn(sx + 20, sy + sh - 34, 110, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_btn(sx + 140, sy + sh - 34, 145, 22, "Optimize RAM (o)", 0x03EA, 0xFFFF);
    draw_btn(sx + 295, sy + sh - 34, 120, 22, "Tasks (t)", 0x24EE, 0xFFFF);
    draw_btn(sx + sw - 100, sy + sh - 34, 80, 22, "Close (c)", 0xF9A6, 0x0000);

    if (optimize_flash) {
        print_string("[*] Cache Flushed!", sx + 430, sy + sh - 28, 0x03EA);
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

    if (mem_view_mode == 1) {
        // Task Manager Mode
        // "Memory View (m)" button
        if (mouse_x >= sx + 20 && mouse_x <= sx + 160 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            mem_view_mode = 0;
            mem_draw();
            play_sound(750); sleep(20); no_sound();
            return 1;
        }

        // "Refresh (r)" button
        if (mouse_x >= sx + 170 && mouse_x <= sx + 280 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            mem_draw();
            play_sound(750); sleep(20); no_sound();
            return 1;
        }

        // "Kill Task (k)" button
        if (mouse_x >= sx + 290 && mouse_x <= sx + 410 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            task_info_t tlist[MAX_TASKS];
            int tcount = task_get_list(tlist, MAX_TASKS);
            if (selected_task_idx >= 0 && selected_task_idx < tcount) {
                int pid_to_kill = tlist[selected_task_idx].pid;
                if (pid_to_kill > 0) {
                    int aid = tlist[selected_task_idx].app_id;
                    task_kill(pid_to_kill);
                    if (aid > 0 && aid != MAXP_APP_MEM) {
                        maxp_close_app(aid);
                    }
                    play_sound(400); sleep(40); play_sound(250); sleep(60); no_sound();
                    mem_draw();
                    return 1;
                }
            }
        }

        // Click row in table to select task
        int table_y = sy + 68;
        if (mouse_y >= table_y + 24 && mouse_y <= table_y + 24 + (10 * 20)) {
            int clicked_idx = (mouse_y - (table_y + 24)) / 20;
            task_info_t tlist[MAX_TASKS];
            int tcount = task_get_list(tlist, MAX_TASKS);
            if (clicked_idx >= 0 && clicked_idx < tcount) {
                selected_task_idx = clicked_idx;
                mem_draw();
                play_sound(800); sleep(15); no_sound();
                return 1;
            }
        }
    } else {
        // Memory Monitor Mode
        // Bottom refresh button
        if (mouse_x >= sx + 20 && mouse_x <= sx + 130 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            mem_draw();
            play_sound(750); sleep(30); no_sound();
            return 1;
        }

        // Bottom optimize RAM button
        if (mouse_x >= sx + 140 && mouse_x <= sx + 285 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            optimize_flash = 1;
            play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
            mem_draw();
            return 1;
        }

        // "Tasks (t)" button
        if (mouse_x >= sx + 295 && mouse_x <= sx + 415 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            mem_view_mode = 1;
            mem_draw();
            play_sound(850); sleep(30); no_sound();
            return 1;
        }
    }

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

    // 't' or 'T' switches to Task Manager
    if (ascii_char == 't' || ascii_char == 'T') {
        mem_view_mode = 1;
        mem_draw();
        play_sound(850); sleep(20); no_sound();
        return 1;
    }

    // 'm' or 'M' switches to Memory Monitor
    if (ascii_char == 'm' || ascii_char == 'M') {
        mem_view_mode = 0;
        mem_draw();
        play_sound(750); sleep(20); no_sound();
        return 1;
    }

    // 'r' or 'R' refreshes
    if (ascii_char == 'r' || ascii_char == 'R') {
        mem_draw();
        play_sound(750); sleep(20); no_sound();
        return 1;
    }

    // 'o' or 'O' optimizes in Memory mode
    if (mem_view_mode == 0 && (ascii_char == 'o' || ascii_char == 'O')) {
        optimize_flash = 1;
        play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
        mem_draw();
        return 1;
    }

    // 'k' or 'K' kills selected task in Task Manager mode
    if (mem_view_mode == 1 && (ascii_char == 'k' || ascii_char == 'K')) {
        task_info_t tlist[MAX_TASKS];
        int tcount = task_get_list(tlist, MAX_TASKS);
        if (selected_task_idx >= 0 && selected_task_idx < tcount) {
            int pid_to_kill = tlist[selected_task_idx].pid;
            if (pid_to_kill > 0) {
                int aid = tlist[selected_task_idx].app_id;
                task_kill(pid_to_kill);
                if (aid > 0 && aid != MAXP_APP_MEM) {
                    maxp_close_app(aid);
                }
                play_sound(400); sleep(40); play_sound(250); sleep(60); no_sound();
                mem_draw();
                return 1;
            }
        }
    }

    // Arrow keys or W/S to navigate task selection in Task Manager
    if (mem_view_mode == 1) {
        if (scan_code == 0x48 || ascii_char == 'w' || ascii_char == 'W') { // Up
            if (selected_task_idx > 0) {
                selected_task_idx--;
                mem_draw();
                play_sound(800); sleep(15); no_sound();
            }
            return 1;
        }
        if (scan_code == 0x50 || ascii_char == 's' || ascii_char == 'S') { // Down
            selected_task_idx++;
            mem_draw();
            play_sound(800); sleep(15); no_sound();
            return 1;
        }
    }

    return 0;
}

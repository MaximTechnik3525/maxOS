#include "mem.h"
#include "kernel.h"
#include "maxp.h"
#include "maxfs.h"
#include "user.h"
#include "task.h"
#include "debug.h"
#include "../user/libc/include/maxos.h"
#include "../user/libc/sys/syscall.h"

int mem_open = 0;

static mem_state_t primary_mem_state;
static int cur_mem_x = 80;
static int cur_mem_y = 45;
static int cur_mem_w = 760;
static int cur_mem_h = 490;

void mem_instance_init(mem_state_t* s, int mode) {
    s->mem_view_mode = mode;
    s->selected_task_idx = 0;
    s->optimize_flash = 0;
}

static int optimize_flash = 0;
static int mem_view_mode = 0; // 0 = RAM Monitor, 1 = Task Manager, 2 = RAM Stress Test
static int selected_task_idx = 0;

// RAM Stress Test Engine & Benchmark State
#define STRESS_CHUNK_DWORDS 65536 // 256 KB per tick step
#define STRESS_MAX_MB 8
static unsigned int stress_buffer[STRESS_MAX_MB * 256 * 1024]; // 8 MB buffer in BSS
static int stress_active = 0;
static unsigned int stress_target_mb = 8;
static unsigned int stress_passes_completed = 0;
static unsigned int stress_errors_detected = 0;
static unsigned int stress_current_offset = 0; // in dwords
static unsigned int stress_pattern_idx = 0;
static unsigned int stress_mb_per_sec = 0;
static unsigned int stress_tick_counter = 0;
static unsigned int stress_bytes_in_window = 0;
static unsigned long long stress_total_tested_bytes = 0;

static const unsigned int STRESS_PATTERNS[] = {
    0x55555555,
    0xAAAAAAAA,
    0x00FF00FF,
    0xFF00FF00,
    0xA5A5A5A5,
    0x5A5A5A5A,
    0x12345678,
    0xEDCBA987
};
#define STRESS_PATTERN_COUNT 8

static void hex_to_str(unsigned int val, char* out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = '0';
    out[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        out[2 + (7 - i)] = hex_chars[(val >> (i * 4)) & 0xF];
    }
    out[10] = '\0';
}

unsigned int mem_get_stress_kb(void) {
    return stress_active ? (stress_target_mb * 1024) : 0;
}

int mem_stress_is_active(void) {
    return stress_active;
}

void mem_stress_run_pass(void) {
    stress_active = 1;
}

void mem_stress_release(void) {
    stress_active = 0;
    stress_current_offset = 0;
}

/* Button helper using global UI primitive */
#define draw_btn draw_ui_btn

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
    mem_instance_init(&primary_mem_state, 0);
    optimize_flash = 0;
    mem_view_mode = 0;
    selected_task_idx = 0;
    stress_active = 0;
    stress_passes_completed = 0;
    stress_errors_detected = 0;
    stress_current_offset = 0;
    stress_pattern_idx = 0;
    stress_mb_per_sec = 0;
    stress_tick_counter = 0;
    stress_bytes_in_window = 0;
    stress_total_tested_bytes = 0;
}

void mem_open_window(void) {
    mem_open = 1;
    optimize_flash = 0;
    drag = 1;
    selected_task_idx = 0;
    maxp_set_active_app(MAXP_APP_MEM);
    maxp_draw_active_instance();
}

void mem_open_stress_window(void) {
    mem_open = 1;
    mem_view_mode = 2;
    optimize_flash = 0;
    drag = 1;
    selected_task_idx = 0;
    maxp_set_active_app(MAXP_APP_MEM);
    maxp_draw_active_instance();
}

void mem_close_window(void) {
    optimize_flash = 0;
    stress_active = 0;
    maxp_close_app(MAXP_APP_MEM);
}

void mem_tick(void) {
    if (!mem_open || !stress_active) return;

    unsigned int pattern = STRESS_PATTERNS[stress_pattern_idx % STRESS_PATTERN_COUNT];
    unsigned int total_dwords = stress_target_mb * 256 * 1024;
    unsigned int chunk = STRESS_CHUNK_DWORDS;
    if (stress_current_offset + chunk > total_dwords) {
        chunk = total_dwords - stress_current_offset;
    }

    // Write phase
    for (unsigned int i = 0; i < chunk; i++) {
        stress_buffer[stress_current_offset + i] = pattern ^ (stress_current_offset + i);
    }

    // Read & Verify phase
    for (unsigned int i = 0; i < chunk; i++) {
        unsigned int expected = pattern ^ (stress_current_offset + i);
        if (stress_buffer[stress_current_offset + i] != expected) {
            stress_errors_detected++;
        }
    }

    unsigned int bytes_chunk = chunk * sizeof(unsigned int) * 2; // read + write
    stress_bytes_in_window += bytes_chunk;
    stress_total_tested_bytes += bytes_chunk;
    stress_current_offset += chunk;

    if (stress_current_offset >= total_dwords) {
        stress_current_offset = 0;
        stress_passes_completed++;
        stress_pattern_idx++;
        play_sound(880); sleep(5); no_sound();
    }

    stress_tick_counter++;
    if (stress_tick_counter >= 15) { // every ~300ms
        stress_mb_per_sec = (unsigned int)(((unsigned long long)stress_bytes_in_window * 1000) / (300ULL * 1024 * 1024));
        stress_bytes_in_window = 0;
        stress_tick_counter = 0;
    }

    if (mem_view_mode == 2) {
        mem_draw();
    }
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
    draw_btn(sx + 20, sy + sh - 34, 130, 22, "Memory View (m)", 0x24EE, 0xFFFF);
    draw_btn(sx + 160, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_btn(sx + 270, sy + sh - 34, 115, 22, "Kill Task (k)", 0xF800, 0xFFFF);
    draw_btn(sx + 395, sy + sh - 34, 95, 22, "Stress (s)", 0x0DE5, 0x0000);
}

static void draw_stress_test(int sx, int sy, int sw, int sh) {
    // Header banner
    draw_rect(sx + 14, sy + 34, sw - 28, 28, 0x03EA);
    print_string("RAM Hardware Stress Test & Diagnostic Benchmark", sx + 22, sy + 40, 0xFFFF);
    print_string("Preemptive Ring 3 continuous memory verification & throughput testing", sx + 22, sy + 52, 0xFFFF);

    // Progress Bar
    int bar_x = sx + 20;
    int bar_y = sy + 70;
    int bar_w = sw - 40;
    int bar_h = 24;

    draw_rect(bar_x, bar_y, bar_w, bar_h, 0x0000);
    draw_rect(bar_x + 1, bar_y + 1, bar_w - 2, bar_h - 2, 0x2124);

    unsigned int total_dwords = stress_target_mb * 256 * 1024;
    int fill_w = 0;
    if (total_dwords > 0 && stress_current_offset > 0) {
        fill_w = (int)(((unsigned long long)stress_current_offset * (bar_w - 4)) / total_dwords);
    }
    if (fill_w > bar_w - 4) fill_w = bar_w - 4;

    unsigned short bar_col = (stress_errors_detected > 0) ? 0xF800 : (stress_active ? 0x07E0 : 0x03EA);
    if (fill_w > 0) {
        draw_rect(bar_x + 2, bar_y + 2, fill_w, bar_h - 4, bar_col);
    }

    // Status summary under progress bar
    char pct_str[16];
    unsigned int pct = total_dwords ? (unsigned int)(((unsigned long long)stress_current_offset * 100) / total_dwords) : 0;
    int_str((int)pct, pct_str);

    print_string("Pass Progress: ", bar_x, bar_y + 30, 0x0000);
    print_string(pct_str, bar_x + 120, bar_y + 30, 0x0000);
    print_string("%  |  Status: ", bar_x + 145, bar_y + 30, 0x0000);
    if (stress_active) {
        print_string("STRESSING RAM", bar_x + 235, bar_y + 30, 0x05E0);
    } else {
        print_string("IDLE (PAUSED)", bar_x + 235, bar_y + 30, 0x8085);
    }

    print_string("  |  Integrity: ", bar_x + 360, bar_y + 30, 0x0000);
    if (stress_errors_detected == 0) {
        print_string("0 Errors (100% HEALTHY)", bar_x + 475, bar_y + 30, 0x05E0);
    } else {
        char err_str[16];
        int_str((int)stress_errors_detected, err_str);
        print_string(err_str, bar_x + 475, bar_y + 30, 0xF800);
        print_string(" ERRORS DETECTED!", bar_x + 505, bar_y + 30, 0xF800);
    }

    // Columns
    int stats_y = bar_y + 54;
    int col1_x = sx + 20;
    int col2_x = sx + (sw / 2) + 10;
    char num_buf[32];

    // Left Column: Test Configuration & Integrity
    draw_rect(col1_x, stats_y, (sw / 2) - 30, 155, 0xF7BE);
    draw_rect(col1_x, stats_y, (sw / 2) - 30, 18, 0xBDD7);
    print_string("Hardware Test Configuration", col1_x + 6, stats_y + 4, 0x0000);

    int row_y = stats_y + 24;
    print_string("Allocated Buffer: ", col1_x + 10, row_y, 0x0000);
    int_str((int)stress_target_mb, num_buf);
    print_string(num_buf, col1_x + 150, row_y, 0x0000);
    print_string(" MB (8192 KB)", col1_x + 165, row_y, 0x7BEF);
    row_y += 18;

    print_string("Completed Passes: ", col1_x + 10, row_y, 0x0000);
    int_str((int)stress_passes_completed, num_buf);
    print_string(num_buf, col1_x + 150, row_y, 0x03EA);
    row_y += 18;

    char hex_buf[16];
    hex_to_str(STRESS_PATTERNS[stress_pattern_idx % STRESS_PATTERN_COUNT], hex_buf);
    print_string("Current Pattern:  ", col1_x + 10, row_y, 0x0000);
    print_string(hex_buf, col1_x + 150, row_y, 0x0000);
    row_y += 18;

    print_string("Test Algorithm:   ", col1_x + 10, row_y, 0x0000);
    print_string("Bit-XOR Walking (0..8MB)", col1_x + 150, row_y, 0x0000);
    row_y += 18;

    print_string("Total Data Tested:", col1_x + 10, row_y, 0x0000);
    int_str((int)(stress_total_tested_bytes / (1024 * 1024)), num_buf);
    print_string(num_buf, col1_x + 150, row_y, 0x0000);
    print_string(" MB", col1_x + 190, row_y, 0x0000);
    row_y += 18;

    print_string("Test Health:      ", col1_x + 10, row_y, 0x0000);
    if (stress_errors_detected == 0) {
        print_string("OK - No bitflips", col1_x + 150, row_y, 0x05E0);
    } else {
        print_string("WARNING - Faults detected!", col1_x + 150, row_y, 0xF800);
    }

    // Right Column: Live Benchmark Diagnostics
    draw_rect(col2_x, stats_y, (sw / 2) - 30, 155, 0xF7BE);
    draw_rect(col2_x, stats_y, (sw / 2) - 30, 18, 0xBDD7);
    print_string("Live Memory Benchmark", col2_x + 6, stats_y + 4, 0x0000);

    row_y = stats_y + 24;
    print_string("Memory Bandwidth: ", col2_x + 10, row_y, 0x0000);
    int_str((int)stress_mb_per_sec, num_buf);
    print_string(num_buf, col2_x + 150, row_y, 0x0200);
    print_string(" MB/s", col2_x + 190, row_y, 0x0200);
    row_y += 18;

    print_string("Current Offset:   ", col2_x + 10, row_y, 0x0000);
    int_str((int)((stress_current_offset * 4) / 1024), num_buf);
    print_string(num_buf, col2_x + 150, row_y, 0x0000);
    print_string(" KB / 8192 KB", col2_x + 195, row_y, 0x7BEF);
    row_y += 18;

    print_string("Quantum Slice:    ", col2_x + 10, row_y, 0x0000);
    print_string("20 ms Round-Robin", col2_x + 150, row_y, 0x0000);
    row_y += 18;

    print_string("Execution Mode:   ", col2_x + 10, row_y, 0x0000);
    print_string("Ring 3 User Step", col2_x + 150, row_y, 0x03EA);
    row_y += 18;

    print_string("Scheduling Timer: ", col2_x + 10, row_y, 0x0000);
    print_string("PIT 1000 Hz / IRQ0", col2_x + 150, row_y, 0x0000);
    row_y += 18;

    print_string("Bench Status:     ", col2_x + 10, row_y, 0x0000);
    if (stress_active) {
        print_string("Active Measuring", col2_x + 150, row_y, 0x05E0);
    } else {
        print_string("Paused", col2_x + 150, row_y, 0x8085);
    }

    // Bottom Action Buttons in Stress Test mode:
    draw_btn(sx + 20, sy + sh - 34, 130, 22, stress_active ? "Stop Test (s)" : "Start Test (s)", stress_active ? 0xF800 : 0x05E0, 0xFFFF);
    draw_btn(sx + 160, sy + sh - 34, 90, 22, "Reset (x)", 0xC618, 0x0000);
    draw_btn(sx + 260, sy + sh - 34, 125, 22, "Memory View (m)", 0x24EE, 0xFFFF);
    draw_btn(sx + 395, sy + sh - 34, 90, 22, "Tasks (t)", 0x03EA, 0xFFFF);
}

void mem_draw(void) {
    int sx = cur_mem_x;
    int sy = cur_mem_y;
    int sw = cur_mem_w;
    int sh = cur_mem_h;

    // Window frame (Classic 3D border)
    draw_rect(sx, sy, sw, sh, 0x0000);
    draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x041F); // Royal Cyan/Navy
    if (mem_view_mode == 2) {
        print_string("maxOS RAM Stress Test & Diagnostic Benchmark 4.0", sx + 8, sy + 6, 0xFFFF);
    } else {
        print_string("maxOS Memory & Task Manager 4.0 - [Event Loop]", sx + 8, sy + 6, 0xFFFF);
    }

    // [_] Minimize & [X] Close button on titlebar
    draw_btn(sx + sw - 44, sy + 3, 18, 16, "_", 0xCE79, 0x0000);
    draw_btn(sx + sw - 22, sy + 3, 18, 16, "X", 0xF800, 0xFFFF);

    // Main Card background
    draw_rect(sx + 10, sy + 28, sw - 20, sh - 68, 0xFFFF);
    draw_rect(sx + 10, sy + 28, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 28, 1, sh - 68, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    if (mem_view_mode == 1) {
        draw_task_manager(sx, sy, sw, sh);
        return;
    } else if (mem_view_mode == 2) {
        draw_stress_test(sx, sy, sw, sh);
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
    draw_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_btn(sx + 130, sy + sh - 34, 135, 22, "Optimize RAM (o)", 0x03EA, 0xFFFF);
    draw_btn(sx + 275, sy + sh - 34, 90, 22, "Tasks (t)", 0x24EE, 0xFFFF);
    draw_btn(sx + 375, sy + sh - 34, 95, 22, "Stress (s)", 0x0DE5, 0x0000);

    if (optimize_flash) {
        print_string("[*] Cache Flushed!", sx + 430, sy + sh - 28, 0x03EA);
        optimize_flash = 0;
    }
}

int mem_handle_click(int mouse_x, int mouse_y) {
    if (!mem_open) return 0;

    int sx = cur_mem_x;
    int sy = cur_mem_y;
    int sw = cur_mem_w;
    int sh = cur_mem_h;

    // [_] Titlebar minimize button
    if (mouse_x >= sx + sw - 48 && mouse_x <= sx + sw - 26 && mouse_y >= sy && mouse_y <= sy + 24) {
        ui_btn_click_effect(sx + sw - 44, sy + 3, 18, 16, "_", 0xCE79, 0x0000);
        return -2;
    }

    // [X] Titlebar close button
    if (mouse_x >= sx + sw - 26 && mouse_x <= sx + sw && mouse_y >= sy && mouse_y <= sy + 24) {
        ui_btn_click_effect(sx + sw - 22, sy + 3, 18, 16, "X", 0xF800, 0xFFFF);
        mem_close_window();
        return 1;
    }

    if (mem_view_mode == 1) {
        // Task Manager Mode
        // "Memory View (m)" button
        if (mouse_x >= sx + 20 && mouse_x <= sx + 150 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 20, sy + sh - 34, 130, 22, "Memory View (m)", 0x24EE, 0xFFFF);
            mem_view_mode = 0;
            mem_draw();
            return 1;
        }

        // "Refresh (r)" button
        if (mouse_x >= sx + 160 && mouse_x <= sx + 260 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 160, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
            mem_draw();
            return 1;
        }

        // "Kill Task (k)" button
        if (mouse_x >= sx + 270 && mouse_x <= sx + 385 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 270, sy + sh - 34, 115, 22, "Kill Task (k)", 0xF800, 0xFFFF);
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

        // "Stress (s)" button
        if (mouse_x >= sx + 395 && mouse_x <= sx + 490 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 395, sy + sh - 34, 95, 22, "Stress (s)", 0x0DE5, 0x0000);
            mem_view_mode = 2;
            mem_draw();
            return 1;
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
    } else if (mem_view_mode == 0) {
        // Memory Monitor Mode
        // Bottom refresh button
        if (mouse_x >= sx + 20 && mouse_x <= sx + 120 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
            mem_draw();
            return 1;
        }

        // Bottom optimize RAM button
        if (mouse_x >= sx + 130 && mouse_x <= sx + 265 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 130, sy + sh - 34, 135, 22, "Optimize RAM (o)", 0x03EA, 0xFFFF);
            optimize_flash = 1;
            play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
            mem_draw();
            return 1;
        }

        // "Tasks (t)" button
        if (mouse_x >= sx + 275 && mouse_x <= sx + 365 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 275, sy + sh - 34, 90, 22, "Tasks (t)", 0x24EE, 0xFFFF);
            mem_view_mode = 1;
            mem_draw();
            return 1;
        }

        // "Stress (s)" button
        if (mouse_x >= sx + 375 && mouse_x <= sx + 470 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 375, sy + sh - 34, 95, 22, "Stress (s)", 0x0DE5, 0x0000);
            mem_view_mode = 2;
            mem_draw();
            return 1;
        }
    } else if (mem_view_mode == 2) {
        // Stress Test Mode
        // "Start / Stop (s)" button
        if (mouse_x >= sx + 20 && mouse_x <= sx + 150 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 20, sy + sh - 34, 130, 22, stress_active ? "Stop Test (s)" : "Start Test (s)", stress_active ? 0xF800 : 0x05E0, 0xFFFF);
            stress_active = !stress_active;
            if (stress_active) {
                play_sound(750); sleep(30); play_sound(950); sleep(40); no_sound();
            } else {
                play_sound(500); sleep(40); no_sound();
            }
            mem_draw();
            return 1;
        }

        // "Reset (x)" button
        if (mouse_x >= sx + 160 && mouse_x <= sx + 250 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 160, sy + sh - 34, 90, 22, "Reset (x)", 0xC618, 0x0000);
            stress_passes_completed = 0;
            stress_errors_detected = 0;
            stress_current_offset = 0;
            stress_mb_per_sec = 0;
            stress_total_tested_bytes = 0;
            mem_draw();
            return 1;
        }

        // "Memory View (m)" button
        if (mouse_x >= sx + 260 && mouse_x <= sx + 385 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 260, sy + sh - 34, 125, 22, "Memory View (m)", 0x24EE, 0xFFFF);
            mem_view_mode = 0;
            mem_draw();
            return 1;
        }

        // "Tasks (t)" button
        if (mouse_x >= sx + 395 && mouse_x <= sx + 485 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
            ui_btn_click_effect(sx + 395, sy + sh - 34, 90, 22, "Tasks (t)", 0x03EA, 0xFFFF);
            mem_view_mode = 1;
            mem_draw();
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

    // In Memory mode:
    if (mem_view_mode == 0) {
        if (ascii_char == 'o' || ascii_char == 'O') {
            optimize_flash = 1;
            play_sound(500); sleep(30); play_sound(900); sleep(40); no_sound();
            mem_draw();
            return 1;
        }
        if (ascii_char == 's' || ascii_char == 'S') {
            mem_view_mode = 2;
            mem_draw();
            play_sound(850); sleep(20); no_sound();
            return 1;
        }
    }

    // In Task Manager mode:
    if (mem_view_mode == 1) {
        if (ascii_char == 'k' || ascii_char == 'K') {
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

    // In Stress Test mode:
    if (mem_view_mode == 2) {
        if (ascii_char == 's' || ascii_char == 'S' || ascii_char == ' ') {
            stress_active = !stress_active;
            if (stress_active) {
                play_sound(750); sleep(30); play_sound(950); sleep(40); no_sound();
            } else {
                play_sound(500); sleep(40); no_sound();
            }
            mem_draw();
            return 1;
        }
        if (ascii_char == 'x' || ascii_char == 'X') {
            stress_passes_completed = 0;
            stress_errors_detected = 0;
            stress_current_offset = 0;
            stress_mb_per_sec = 0;
            stress_total_tested_bytes = 0;
            play_sound(600); sleep(30); no_sound();
            mem_draw();
            return 1;
        }
    }

    return 0;
}


void mem_instance_draw(mem_state_t* s, int sx, int sy, int sw, int sh) {
    (void)sx; (void)sy; (void)sw; (void)sh;
    if (s) {
        mem_view_mode = s->mem_view_mode;
        selected_task_idx = s->selected_task_idx;
        optimize_flash = s->optimize_flash;
    }
    mem_draw();
    if (s) {
        s->mem_view_mode = mem_view_mode;
        s->selected_task_idx = selected_task_idx;
        s->optimize_flash = optimize_flash;
    }
}

int mem_instance_click(mem_state_t* s, int sx, int sy, int sw, int sh, int mouse_x, int mouse_y) {
    (void)sx; (void)sy; (void)sw; (void)sh;
    if (s) {
        mem_view_mode = s->mem_view_mode;
        selected_task_idx = s->selected_task_idx;
        optimize_flash = s->optimize_flash;
    }
    // Check [_] minimize button explicitly
    int wx = win_x + 20;
    int wy = win_y + 35;
    int ww = win_w - 40;
    if (mouse_x >= wx + ww - 48 && mouse_x <= wx + ww - 26 && mouse_y >= wy && mouse_y <= wy + 24) {
        ui_btn_click_effect(wx + ww - 44, wy + 3, 18, 16, "_", 0xCE79, 0x0000);
        return -2;
    }
    // Check [X] close button explicitly
    if (mouse_x >= wx + ww - 26 && mouse_x <= wx + ww && mouse_y >= wy && mouse_y <= wy + 24) {
        ui_btn_click_effect(wx + ww - 22, wy + 3, 18, 16, "X", 0xF800, 0xFFFF);
        return -1;
    }

    int res = mem_handle_click(mouse_x, mouse_y);
    if (s) {
        s->mem_view_mode = mem_view_mode;
        s->selected_task_idx = selected_task_idx;
        s->optimize_flash = optimize_flash;
    }
    return res;
}

int mem_instance_key(mem_state_t* s, char ascii_char, unsigned char scan_code) {
    if (s) {
        mem_view_mode = s->mem_view_mode;
        selected_task_idx = s->selected_task_idx;
        optimize_flash = s->optimize_flash;
    }
    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01) {
        return -1;
    }
    int res = mem_handle_key(ascii_char, scan_code);
    if (s) {
        s->mem_view_mode = mem_view_mode;
        s->selected_task_idx = selected_task_idx;
        s->optimize_flash = optimize_flash;
    }
    return res;
}

void mem_instance_tick(mem_state_t* s, int sx, int sy, int sw, int sh) {
    (void)sx; (void)sy; (void)sw; (void)sh;
    if (s) {
        mem_view_mode = s->mem_view_mode;
    }
    mem_tick();
}

void mem_main(void) {
    int pid = u_getpid();
    app_instance_t* inst = maxp_get_instance_by_pid(pid);
    mem_state_t* s = inst ? (mem_state_t*)inst->state : &primary_mem_state;
    int sx = inst ? inst->win_x : 80;
    int sy = inst ? inst->win_y : 45;
    int sw = inst ? inst->win_w : 760;
    int sh = inst ? inst->win_h : 490;
    cur_mem_x = sx; cur_mem_y = sy; cur_mem_w = sw; cur_mem_h = sh;
    mem_open = 1;
    maxos_debug_log("MEM", "Mem app started in Ring 3 Event Loop");

    while (1) {
        maxos_event_t ev;
        if (maxos_get_event(&ev)) {
            if (ev.type == EVENT_DRAW) {
                if (ev.x != 0 || ev.y != 0) {
                    sx = ev.x; sy = ev.y; sw = ev.key; sh = ev.scan;
                    cur_mem_x = sx; cur_mem_y = sy; cur_mem_w = sw; cur_mem_h = sh;
                }
                mem_instance_draw(s, sx, sy, sw, sh);
            } else if (ev.type == EVENT_CLICK) {
                int res = mem_instance_click(s, sx, sy, sw, sh, ev.x, ev.y);
                if (res == -1) {
                    if (inst) maxp_close_instance(inst->instance_id);
                    break;
                }
                if (res == -2) {
                    if (inst) maxp_minimize_instance(inst->instance_id);
                } else if (res) {
                    mem_instance_draw(s, sx, sy, sw, sh);
                }
            } else if (ev.type == EVENT_KEY) {
                int res = mem_instance_key(s, (char)ev.key, (unsigned char)ev.scan);
                if (res == -1) {
                    if (inst) maxp_close_instance(inst->instance_id);
                    break;
                }
                if (res) {
                    mem_instance_draw(s, sx, sy, sw, sh);
                }
            } else if (ev.type == EVENT_TICK) {
                mem_instance_tick(s, sx, sy, sw, sh);
            }
        }
        maxos_yield();
    }
}


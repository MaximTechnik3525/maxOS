#include "sysinfo.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"
#include "kernel.h"
#include "user.h"
#include "debug.h"

void get_cpu(char* buffer);

int sysinfo_open = 0;

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 6, by + 4, text_col);
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
    maxp_close_app(MAXP_APP_SYSINFO);
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
    draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x0DE5);
    print_string("maxOS System Hardware & Kernel Information (SysInfo)", sx + 8, sy + 6, 0x0000);

    // Close button
    draw_ui_btn(sx + sw - 56, sy + 3, 50, 18, "Close", 0xF9A6, 0x0000);

    // Main Card
    draw_rect(sx + 10, sy + 30, sw - 20, sh - 70, 0xFFFF);
    draw_rect(sx + 10, sy + 30, sw - 20, 1, 0x7BEF);
    draw_rect(sx + 10, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + sw - 11, sy + 30, 1, sh - 70, 0x7BEF);
    draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    // Section 1: OS Banner
    draw_rect(sx + 15, sy + 38, sw - 30, 32, 0x0DE5);
    print_string("maxOS RedCycle v3.1 (x86_64 Long Mode)", sx + 25, sy + 44, 0x0000);
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
    print_string(numbuf, sx + 200, row_y, 0x0200);
    print_string(" Writes=", sx + 245, row_y, 0x0000);
    int_str((int)stats->writes_count, numbuf);
    print_string(numbuf, sx + 320, row_y, 0x0200);
    print_string(" Flushes=", sx + 355, row_y, 0x0000);
    int_str((int)stats->flushes_count, numbuf);
    print_string(numbuf, sx + 440, row_y, 0x0200);
    print_string(" Errors=", sx + 475, row_y, 0x0000);
    int_str((int)stats->errors_count, numbuf);
    print_string(numbuf, sx + 545, row_y, (stats->errors_count > 0) ? 0xF800 : 0x0200);
    row_y += 15;

    print_string("Last ATA Operation: ", sx + 25, row_y, 0x0000);
    print_string((char*)stats->last_op, sx + 210, row_y, 0x24EE);
    print_string(" | LBA: ", sx + 275, row_y, 0x0000);
    int_str((int)stats->last_lba, numbuf);
    print_string(numbuf, sx + 345, row_y, 0x0000);
    print_string(" | Status Reg: 0x", sx + 410, row_y, 0x0000);
    int_str((int)stats->last_status, numbuf);
    print_string(numbuf, sx + 565, row_y, 0x0000);
    row_y += 24;

    // Section 5: Kernel & Application Debug Log (Circular Ring Buffer)
    print_string("[ Diagnostic Kernel & App Log (COM1) ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    int count = debug_history_total;
    if (count > 4) count = 4;
    if (count == 0) {
        print_string("No log entries recorded yet.", sx + 25, row_y, 0x7BEF);
    } else {
        for (int i = 0; i < count; i++) {
            const struct DebugLogEntry* ent = debug_get_entry(debug_history_total - count + i);
            if (ent) {
                print_string("[", sx + 25, row_y, 0x7BEF);
                print_string((char*)ent->tag, sx + 32, row_y, 0x03EA);
                print_string("] ", sx + 62, row_y, 0x7BEF);
                print_string((char*)ent->msg, sx + 75, row_y, 0x0000);
                row_y += 15;
            }
        }
    }

    // Bottom action buttons
    draw_ui_btn(sx + 20, sy + sh - 34, 80, 22, "Refresh", 0xC618, 0x0000);
    draw_ui_btn(sx + 110, sy + sh - 34, 130, 22, "Ring 3 Demo (u)", 0x03EA, 0xFFFF);
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

    // Bottom Ring 3 Demo button
    if (mouse_x >= sx + 110 && mouse_x <= sx + 240 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
        ring3_demo_launch();
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

    // 'u' or 'U' launches Ring 3 demo
    if (ascii_char == 'u' || ascii_char == 'U') {
        ring3_demo_launch();
        return 1;
    }

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

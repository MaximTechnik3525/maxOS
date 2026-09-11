#include "sysinfo.h"
#include "../user/libc/include/maxos.h"
#include "../user/libc/include/string.h"
#include "maxp.h"
#include "ata.h"
#include "task.h"

extern void get_cpu(char* buffer);
extern long get_cpl(void);
extern void int_str(int n, char* buf);

int sysinfo_open = 0;

static void draw_ui_btn(int x, int y, int w, int h, const char* label, unsigned short fill, unsigned short text_col) {
    maxos_draw_rect(x, y, w, h, 0x0000);
    maxos_draw_rect(x + 1, y + 1, w - 2, h - 2, fill);
    maxos_print_text(label, x + (w / 2) - (strlen(label) * 4), y + (h / 2) - 8, text_col);
}

static void sysinfo_render(int sx, int sy, int sw, int sh) {
    maxos_draw_rect(sx, sy, sw, sh, 0x0000);
    maxos_draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    maxos_draw_rect(sx + 2, sy + 2, sw - 4, 20, 0x0DE5);
    maxos_print_text("maxOS System Hardware & Kernel Information", sx + 8, sy + 6, 0x0000);

    draw_ui_btn(sx + sw - 44, sy + 3, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(sx + sw - 22, sy + 3, 18, 16, "X", 0xF800, 0xFFFF);

    maxos_draw_rect(sx + 10, sy + 30, sw - 20, sh - 70, 0xFFFF);
    maxos_draw_rect(sx + 10, sy + 30, sw - 20, 1, 0x7BEF);
    maxos_draw_rect(sx + 10, sy + 30, 1, sh - 70, 0x7BEF);
    maxos_draw_rect(sx + sw - 11, sy + 30, 1, sh - 70, 0x7BEF);
    maxos_draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    maxos_draw_rect(sx + 15, sy + 38, sw - 30, 32, 0x0DE5);
    maxos_print_text("maxOS MaxRing v3.5 (x86_64 Long Mode)", sx + 25, sy + 44, 0x0000);
    maxos_print_text("Kernel Architecture: 64-bit AMD64 / Intel 64", sx + 25, sy + 56, 0x0000);

    int row_y = sy + 82;

    maxos_print_text("[ Processor / CPU ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    char cpu_model[49];
    get_cpu(cpu_model);
    maxos_print_text("Model: ", sx + 25, row_y, 0x0000);
    maxos_print_text(cpu_model, sx + 80, row_y, 0x24EE);
    row_y += 15;
    maxos_print_text("Mode:  64-bit Long Mode | PML4 Paging | System V AMD64 ABI", sx + 25, row_y, 0x0000);
    row_y += 15;
    maxos_print_text("Interrupts: 64-bit IDT Active | 8259 PIC Remapped | PIT 1000Hz Timer", sx + 25, row_y, 0x03EA);
    row_y += 15;
    long current_cpl = get_cpl();
    if (current_cpl == 3) {
        maxos_print_text("Privilege: Ring 3 User Mode (CPL = 3) [ALL APPS PROTECTED]", sx + 25, row_y, 0x03EA);
    } else {
        maxos_print_text("Privilege: Ring 0 Kernel Mode (User Mode Ready)", sx + 25, row_y, 0x03EA);
    }
    row_y += 24;

    maxos_print_text("[ Display & Memory ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    maxos_print_text("Graphics: VESA VBE 1024x768 @ 16-bit HighColor (RGB 565)", sx + 25, row_y, 0x0000);
    row_y += 15;
    maxos_print_text("Linear Framebuffer Address: 0xFD000000 (Identity Mapped)", sx + 25, row_y, 0x0000);
    row_y += 15;
    maxos_print_text("Kernel Load Address:        0x00100000 (1 MB Physical RAM)", sx + 25, row_y, 0x0000);
    row_y += 24;

    maxos_print_text("[ Storage & ATA Driver Diagnostics ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    maxos_print_text("Storage Device: ATA Primary Master (IDE PIO)", sx + 25, row_y, 0x0000);
    row_y += 15;
    maxos_print_text("Drive Model: ", sx + 25, row_y, 0x0000);
    maxos_print_text(ata_primary_master.present ? ata_primary_master.model : "Not Detected", sx + 135, row_y, 0x24EE);
    row_y += 15;

    char numbuf[32];
    maxos_print_text("Capacity: ", sx + 25, row_y, 0x0000);
    int_str((int)ata_primary_master.size_mb, numbuf);
    maxos_print_text(numbuf, sx + 115, row_y, 0x03EA);
    maxos_print_text(" MB (", sx + 155, row_y, 0x0000);
    int_str((int)ata_primary_master.total_sectors, numbuf);
    maxos_print_text(numbuf, sx + 200, row_y, 0x03EA);
    maxos_print_text(" sectors) | maxFS 2.0 Inode FS", sx + 280, row_y, 0x0000);
    row_y += 15;

    const struct ATADebugStats* stats = ata_get_debug_stats();
    maxos_print_text("IO Counters: Reads=", sx + 25, row_y, 0x0000);
    int_str((int)stats->reads_count, numbuf);
    maxos_print_text(numbuf, sx + 195, row_y, 0x0000);
    maxos_print_text(" | Writes=", sx + 245, row_y, 0x0000);
    int_str((int)stats->writes_count, numbuf);
    maxos_print_text(numbuf, sx + 325, row_y, 0x0000);
    maxos_print_text(" | Errors=", sx + 375, row_y, 0x0000);
    int_str((int)stats->errors_count, numbuf);
    maxos_print_text(numbuf, sx + 450, row_y, (stats->errors_count == 0) ? 0x05E0 : 0xF800);

    row_y += 24;
    maxos_print_text("[ Preemptive Multitasking & Processes ]", sx + 20, row_y, 0x11EB);
    row_y += 16;
    maxos_print_text("Scheduler: Round-Robin 20ms Time Slices | 15 CPU Registers Saved | Multi-Instance", sx + 25, row_y, 0x0000);
    row_y += 15;
    int tcount = task_get_count();
    maxos_print_text("Active Processes: ", sx + 25, row_y, 0x0000);
    int_str(tcount, numbuf);
    maxos_print_text(numbuf, sx + 180, row_y, 0x05E0);
    maxos_print_text(" running simultaneously", sx + 205, row_y, 0x0000);

    draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0x03EA, 0xFFFF);
}

void sysinfo_main(void) {
    int sx = 140, sy = 55, sw = 540, sh = 380;
    maxos_debug_log("SYSINFO", "SysInfo app started in Ring 3 Event Loop");

    while (1) {
        maxos_event_t ev;
        if (maxos_get_event(&ev)) {
            if (ev.type == EVENT_DRAW) {
                if (ev.x != 0 || ev.y != 0) {
                    sx = ev.x; sy = ev.y; sw = ev.key; sh = ev.scan;
                }
                sysinfo_render(sx, sy, sw, sh);
            } else if (ev.type == EVENT_CLICK) {
                int mouse_x = ev.x;
                int mouse_y = ev.y;
                
                if (mouse_x >= sx + 20 && mouse_x <= sx + 120 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                    draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xFFFF, 0x0000);
                    maxos_sleep(30);
                    sysinfo_render(sx, sy, sw, sh);
                }

                if (mouse_x >= sx + 130 && mouse_x <= sx + 310 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                    draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0xFFFF, 0x0000);
                    maxos_sleep(30);
                    draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0x03EA, 0xFFFF);
                }
            } else if (ev.type == EVENT_KEY) {
                if (ev.key == 'r' || ev.key == 'R') {
                    sysinfo_render(sx, sy, sw, sh);
                }
            }
        }
        maxos_yield();
    }
}

void sysinfo_init(void) {}
void sysinfo_open_window(void) { maxp_spawn_instance(MAXP_APP_SYSINFO, "SysInfo", 0); }
void sysinfo_close_window(void) {}
void sysinfo_draw(void) {}
int sysinfo_handle_click(int mx, int my) { return 0; }
int sysinfo_handle_key(char ch, unsigned char scan) { return 0; }

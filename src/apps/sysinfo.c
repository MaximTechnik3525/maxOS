#include "sysinfo.h"
#include "../user/libc/include/maxos.h"
#include "string.h"
#include "maxp.h"
#include "ata.h"
#include "ahci.h"
#include "pci.h"
#include "task.h"

extern void get_cpu(char* buffer);
extern long get_cpl(void);
extern void int_str(int n, char* buf);

int sysinfo_open = 0;
static int sysinfo_tab = 0; // 0 = System Overview, 1 = PCI Device Manager
static int pci_page = 0;

static void draw_ui_btn(int x, int y, int w, int h, const char* label, unsigned short fill, unsigned short text_col) {
    maxos_draw_rect(x, y, w, h, 0x0000);
    maxos_draw_rect(x + 1, y + 1, w - 2, h - 2, fill);
    int len = strlen(label);
    int tx = x + (w - len * 9) / 2;
    int ty = y + (h - 8) / 2;
    maxos_print_text(label, tx, ty, text_col);
}

static void hex_to_str16(unsigned short val, char* out) {
    const char* hex = "0123456789ABCDEF";
    out[0] = '0';
    out[1] = 'x';
    out[2] = hex[(val >> 12) & 0xF];
    out[3] = hex[(val >> 8) & 0xF];
    out[4] = hex[(val >> 4) & 0xF];
    out[5] = hex[val & 0xF];
    out[6] = '\0';
}

static void hex_to_str32(unsigned int val, char* out) {
    const char* hex = "0123456789ABCDEF";
    out[0] = '0';
    out[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        out[2 + i] = hex[val & 0xF];
        val >>= 4;
    }
    out[10] = '\0';
}

static void sysinfo_render_overview(int sx, int sy, int sw, int sh) {
    (void)sh;
    maxos_draw_rect(sx + 15, sy + 54, sw - 30, 26, 0x0DE5);
    maxos_print_text("maxOS v4.0 EventUpdate (x86_64 Long Mode)", sx + 25, sy + 58, 0x0000);
    maxos_print_text("Kernel Architecture: 64-bit AMD64 | Event Loop Apps", sx + 25, sy + 68, 0x0000);

    int row_y = sy + 86;

    maxos_print_text("[ Processor / CPU ]", sx + 20, row_y, 0x11EB);
    row_y += 14;
    char cpu_model[49];
    get_cpu(cpu_model);
    maxos_print_text("Model: ", sx + 25, row_y, 0x0000);
    maxos_print_text(cpu_model, sx + 80, row_y, 0x24EE);
    row_y += 13;
    maxos_print_text("Mode:  64-bit Long Mode | PML4 Paging | System V AMD64 ABI", sx + 25, row_y, 0x0000);
    row_y += 13;
    maxos_print_text("Interrupts: 64-bit IDT Active | 8259 PIC Remapped | PIT 1000Hz Timer", sx + 25, row_y, 0x03EA);
    row_y += 13;
    long current_cpl = get_cpl();
    if (current_cpl == 3) {
        maxos_print_text("Privilege: Ring 3 User Mode (CPL = 3) [ALL APPS PROTECTED]", sx + 25, row_y, 0x03EA);
    } else {
        maxos_print_text("Privilege: Ring 0 Kernel Mode (User Mode Ready)", sx + 25, row_y, 0x03EA);
    }
    row_y += 17;

    maxos_print_text("[ Display & Memory ]", sx + 20, row_y, 0x11EB);
    row_y += 14;
    maxos_print_text("Graphics: VESA VBE 1024x768 @ 16-bit HighColor (RGB 565)", sx + 25, row_y, 0x0000);
    row_y += 13;
    maxos_print_text("Linear Framebuffer Address: 0xFD000000 (Identity Mapped)", sx + 25, row_y, 0x0000);
    row_y += 13;
    maxos_print_text("Kernel Load Address:        0x00100000 (1 MB Physical RAM)", sx + 25, row_y, 0x0000);
    row_y += 17;

    maxos_print_text("[ Storage & Disk Controller Diagnostics ]", sx + 20, row_y, 0x11EB);
    row_y += 14;
    if (ahci_is_available()) {
        maxos_print_text("Controller: SATA AHCI 1.0 (Bus Master DMA / MMIO)", sx + 25, row_y, 0x05E0);
    } else {
        char ctrl_str[64];
        strcpy(ctrl_str, "Controller: IDE ");
        strcat(ctrl_str, ata_primary_master.present ? ata_primary_master.channel_name : "None");
        strcat(ctrl_str, " (PIO 28-bit LBA)");
        maxos_print_text(ctrl_str, sx + 25, row_y, 0x0000);
    }
    row_y += 13;
    maxos_print_text("Drive Model: ", sx + 25, row_y, 0x0000);
    maxos_print_text(ata_primary_master.present ? ata_primary_master.model : "Not Detected", sx + 135, row_y, 0x24EE);
    row_y += 13;

    char numbuf[32];
    maxos_print_text("Capacity: ", sx + 25, row_y, 0x0000);
    int_str((int)ata_primary_master.size_mb, numbuf);
    maxos_print_text(numbuf, sx + 115, row_y, 0x03EA);
    maxos_print_text(" MB (", sx + 155, row_y, 0x0000);
    int_str((int)ata_primary_master.total_sectors, numbuf);
    maxos_print_text(numbuf, sx + 200, row_y, 0x03EA);
    maxos_print_text(" sectors) | maxFS 2.0 Inode FS", sx + 280, row_y, 0x0000);
    row_y += 13;

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

    row_y += 17;
    maxos_print_text("[ Preemptive Multitasking & Processes ]", sx + 20, row_y, 0x11EB);
    row_y += 14;
    int tcount = task_get_count();
    maxos_print_text("Active Processes: ", sx + 25, row_y, 0x0000);
    int_str(tcount, numbuf);
    maxos_print_text(numbuf, sx + 180, row_y, 0x05E0);
    maxos_print_text(" running simultaneously in Round-Robin Scheduler", sx + 205, row_y, 0x0000);

    draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xC618, 0x0000);
    draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0x03EA, 0xFFFF);
    draw_ui_btn(sx + 320, sy + sh - 34, 195, 22, "View PCI Devices (2) ->", 0x2417, 0xFFFF);
}

static void sysinfo_render_pci(int sx, int sy, int sw, int sh) {
    int total_devs = pci_get_device_count();
    int devs_per_page = 4;
    int total_pages = (total_devs + devs_per_page - 1) / devs_per_page;
    if (total_pages <= 0) total_pages = 1;
    if (pci_page >= total_pages) pci_page = total_pages - 1;
    if (pci_page < 0) pci_page = 0;

    // PCI Subtitle banner
    maxos_draw_rect(sx + 15, sy + 54, sw - 30, 20, 0x2417);
    maxos_print_text("PCI Hardware Bus Scanner & Device Manager", sx + 25, sy + 60, 0xFFFF);

    char numbuf[32];
    maxos_print_text("Total PCI Devices Detected: ", sx + 20, sy + 79, 0x0000);
    int_str(total_devs, numbuf);
    maxos_print_text(numbuf, sx + 260, sy + 79, 0x0DE5);
    maxos_print_text(" (Bus 0..31)", sx + 285, sy + 79, 0x8085);

    int start_idx = pci_page * devs_per_page;
    for (int i = 0; i < devs_per_page; i++) {
        int dev_idx = start_idx + i;
        int by = sy + 97 + (i * 56);
        int bw = sw - 30;
        int bh = 51;
        int bx = sx + 15;

        maxos_draw_rect(bx, by, bw, bh, 0xCE79);
        maxos_draw_rect(bx + 1, by + 1, bw - 2, bh - 2, 0xFFFF);

        if (dev_idx < total_devs) {
            pci_device_t* dev = pci_get_device(dev_idx);
            if (dev) {
                // Badge for BDF
                char bdf[24];
                char tmp[8];
                bdf[0] = '\0';
                int_str(dev->bus, tmp);
                if (dev->bus < 10) strcat(bdf, "0");
                strcat(bdf, tmp);
                strcat(bdf, ":");
                int_str(dev->device, tmp);
                if (dev->device < 10) strcat(bdf, "0");
                strcat(bdf, tmp);
                strcat(bdf, ".");
                int_str(dev->func, tmp);
                strcat(bdf, tmp);

                draw_ui_btn(bx + 4, by + 4, 60, 16, bdf, 0x11EB, 0xFFFF);

                // Device Name
                maxos_print_text(dev->device_name, bx + 70, by + 7, 0x0000);
                maxos_print_text(dev->vendor_name, bx + 360, by + 7, 0x24EE);

                // Line 2: Class, ID, IRQ
                char line2[80];
                char vid_str[8], did_str[8];
                hex_to_str16(dev->vendor_id, vid_str);
                hex_to_str16(dev->device_id, did_str);

                maxos_print_text("Class: ", bx + 10, by + 23, 0x8085);
                maxos_print_text(dev->class_name, bx + 65, by + 23, 0x11EB);

                line2[0] = '\0';
                strcat(line2, "ID: ");
                strcat(line2, vid_str);
                strcat(line2, ":");
                strcat(line2, did_str);
                maxos_print_text(line2, bx + 275, by + 23, 0x8085);

                maxos_print_text("IRQ: ", bx + 430, by + 23, 0x8085);
                if (dev->irq > 0 && dev->irq < 255) {
                    int_str(dev->irq, tmp);
                    maxos_print_text(tmp, bx + 470, by + 23, 0x05E0);
                } else {
                    maxos_print_text("None", bx + 470, by + 23, 0x8085);
                }

                // Line 3: Resources / BARs
                int found_bar = 0;
                for (int b = 0; b < 6; b++) {
                    if (dev->bar[b] != 0) {
                        char bar_str[64];
                        char bar_hex[16];
                        hex_to_str32(dev->bar[b], bar_hex);
                        bar_str[0] = '\0';
                        strcat(bar_str, "BAR");
                        int_str(b, tmp);
                        strcat(bar_str, tmp);
                        strcat(bar_str, ": ");
                        strcat(bar_str, bar_hex);
                        strcat(bar_str, dev->bar_is_io[b] ? " (I/O Port)" : " (MMIO)");
                        if (dev->bar_size[b] > 0) {
                            strcat(bar_str, " [");
                            int_str((int)dev->bar_size[b], tmp);
                            strcat(bar_str, tmp);
                            strcat(bar_str, " B]");
                        }
                        maxos_print_text(bar_str, bx + 10, by + 37, 0x03EA);
                        found_bar = 1;
                        break;
                    }
                }
                if (!found_bar) {
                    maxos_print_text("Resources: System Control / Bridge Device (No BARs)", bx + 10, by + 37, 0x8085);
                }
            }
        } else {
            maxos_print_text("- Empty Slot -", bx + 210, by + 20, 0xBDD7);
        }
    }

    // Bottom Navigation Bar
    draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "< Prev Page", (pci_page > 0) ? 0xCE79 : 0xEF59, 0x0000);

    char page_str[32];
    page_str[0] = '\0';
    strcat(page_str, "Page ");
    int_str(pci_page + 1, numbuf);
    strcat(page_str, numbuf);
    strcat(page_str, " of ");
    int_str(total_pages, numbuf);
    strcat(page_str, numbuf);
    maxos_print_text(page_str, sx + 140, sy + sh - 27, 0x0000);

    draw_ui_btn(sx + 240, sy + sh - 34, 100, 22, "Next Page >", (pci_page < total_pages - 1) ? 0xCE79 : 0xEF59, 0x0000);
    draw_ui_btn(sx + 350, sy + sh - 34, 85, 22, "Rescan (r)", 0xC618, 0x0000);
    draw_ui_btn(sx + 445, sy + sh - 34, 75, 22, "Overview", 0x0DE5, 0x0000);
}

static void sysinfo_render(int sx, int sy, int sw, int sh) {
    maxos_draw_rect(sx, sy, sw, sh, 0x0000);
    maxos_draw_rect(sx + 1, sy + 1, sw - 2, sh - 2, 0xEF59);

    // Titlebar
    maxos_draw_rect(sx + 2, sy + 2, sw - 4, 22, 0x0DE5);
    maxos_print_text("maxOS System Hardware & Device Manager", sx + 8, sy + 9, 0x0000);

    draw_ui_btn(sx + sw - 44, sy + 5, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(sx + sw - 22, sy + 5, 18, 16, "X", 0xF800, 0xFFFF);

    // Window Inner Frame
    maxos_draw_rect(sx + 10, sy + 28, sw - 20, sh - 68, 0xFFFF);
    maxos_draw_rect(sx + 10, sy + 28, sw - 20, 1, 0x7BEF);
    maxos_draw_rect(sx + 10, sy + 28, 1, sh - 68, 0x7BEF);
    maxos_draw_rect(sx + sw - 11, sy + 28, 1, sh - 68, 0x7BEF);
    maxos_draw_rect(sx + 10, sy + sh - 41, sw - 20, 1, 0x7BEF);

    // Tab Headers
    draw_ui_btn(sx + 10, sy + 28, 150, 22, "1: Hardware Info", (sysinfo_tab == 0) ? 0x0DE5 : 0xBDD7, (sysinfo_tab == 0) ? 0x0000 : 0x4208);
    draw_ui_btn(sx + 165, sy + 28, 175, 22, "2: PCI Device Manager", (sysinfo_tab == 1) ? 0x0DE5 : 0xBDD7, (sysinfo_tab == 1) ? 0x0000 : 0x4208);

    if (sysinfo_tab == 0) {
        sysinfo_render_overview(sx, sy, sw, sh);
    } else {
        sysinfo_render_pci(sx, sy, sw, sh);
    }
}

void sysinfo_main(void) {
    int sx = 140, sy = 55, sw = 540, sh = 380;
    maxos_debug_log("SYSINFO", "SysInfo app started in Ring 3 Event Loop with PCI Device Manager");

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

                // Tab 1 Click
                if (mouse_x >= sx + 10 && mouse_x <= sx + 160 && mouse_y >= sy + 28 && mouse_y <= sy + 50) {
                    sysinfo_tab = 0;
                    sysinfo_render(sx, sy, sw, sh);
                }
                // Tab 2 Click
                else if (mouse_x >= sx + 165 && mouse_x <= sx + 340 && mouse_y >= sy + 28 && mouse_y <= sy + 50) {
                    sysinfo_tab = 1;
                    sysinfo_render(sx, sy, sw, sh);
                }

                if (sysinfo_tab == 0) {
                    // Refresh
                    if (mouse_x >= sx + 20 && mouse_x <= sx + 120 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        draw_ui_btn(sx + 20, sy + sh - 34, 100, 22, "Refresh (r)", 0xFFFF, 0x0000);
                        maxos_sleep(30);
                        sysinfo_render(sx, sy, sw, sh);
                    }
                    // Run Ring 3 Demo
                    if (mouse_x >= sx + 130 && mouse_x <= sx + 310 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0xFFFF, 0x0000);
                        maxos_sleep(30);
                        draw_ui_btn(sx + 130, sy + sh - 34, 180, 22, "Run Ring 3 Demo (u)", 0x03EA, 0xFFFF);
                    }
                    // View PCI Devices
                    if (mouse_x >= sx + 320 && mouse_x <= sx + 515 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        sysinfo_tab = 1;
                        sysinfo_render(sx, sy, sw, sh);
                    }
                } else {
                    int total_devs = pci_get_device_count();
                    int total_pages = (total_devs + 3) / 4;
                    if (total_pages <= 0) total_pages = 1;

                    // Prev Page
                    if (mouse_x >= sx + 20 && mouse_x <= sx + 120 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        if (pci_page > 0) {
                            pci_page--;
                            sysinfo_render(sx, sy, sw, sh);
                        }
                    }
                    // Next Page
                    if (mouse_x >= sx + 240 && mouse_x <= sx + 340 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        if (pci_page < total_pages - 1) {
                            pci_page++;
                            sysinfo_render(sx, sy, sw, sh);
                        }
                    }
                    // Refresh PCI view
                    if (mouse_x >= sx + 350 && mouse_x <= sx + 435 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        pci_page = 0;
                        sysinfo_render(sx, sy, sw, sh);
                    }
                    // Back to Overview
                    if (mouse_x >= sx + 445 && mouse_x <= sx + 520 && mouse_y >= sy + sh - 34 && mouse_y <= sy + sh - 12) {
                        sysinfo_tab = 0;
                        sysinfo_render(sx, sy, sw, sh);
                    }
                }
            } else if (ev.type == EVENT_KEY) {
                if (ev.key == 'r' || ev.key == 'R') {
                    if (sysinfo_tab == 1) pci_page = 0;
                    sysinfo_render(sx, sy, sw, sh);
                } else if (ev.key == '1') {
                    sysinfo_tab = 0;
                    sysinfo_render(sx, sy, sw, sh);
                } else if (ev.key == '2' || ev.key == 'p' || ev.key == 'P') {
                    sysinfo_tab = 1;
                    sysinfo_render(sx, sy, sw, sh);
                } else if (ev.key == '<' || ev.key == ',' || ev.scan == 0x4B) { // Left
                    if (sysinfo_tab == 1 && pci_page > 0) {
                        pci_page--;
                        sysinfo_render(sx, sy, sw, sh);
                    }
                } else if (ev.key == '>' || ev.key == '.' || ev.scan == 0x4D) { // Right
                    int total_devs = pci_get_device_count();
                    int total_pages = (total_devs + 3) / 4;
                    if (sysinfo_tab == 1 && pci_page < total_pages - 1) {
                        pci_page++;
                        sysinfo_render(sx, sy, sw, sh);
                    }
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
int sysinfo_handle_click(int mx, int my) { (void)mx; (void)my; return 0; }
int sysinfo_handle_key(char ch, unsigned char scan) { (void)ch; (void)scan; return 0; }

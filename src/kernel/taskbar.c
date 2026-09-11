#include "taskbar.h"
#include "maxp.h"
#include "maxfs.h"
#include "notepad.h"
#include "explorer.h"
#include "installer.h"
#include "calc.h"
#include "sysinfo.h"
#include "pong.h"
#include "mem.h"
#include "kernel.h"
#include "user.h"

int start_menu_open = 0;
static int app_minimized = 0;

void taskbar_init(void) {
    start_menu_open = 0;
    app_minimized = 0;
}

int taskbar_is_start_menu_open(void) {
    return start_menu_open;
}

int taskbar_is_app_minimized(void) {
    return app_minimized;
}

void taskbar_set_app_minimized(int min) {
    app_minimized = min;
}

void taskbar_toggle_start_menu(void) {
    start_menu_open = !start_menu_open;
    play_sound(700); sleep(25); no_sound();
    draw_window();
    draw_cursor(pos_x, pos_y);
}

void taskbar_close_start_menu(void) {
    if (start_menu_open) {
        start_menu_open = 0;
        draw_window();
        draw_cursor(pos_x, pos_y);
    }
}

static unsigned char last_rtc_sec = 0, last_rtc_min = 0, last_rtc_hour = 12;

static unsigned char read_rtc(unsigned char reg) {
    if (get_cpl() == 3) {
        if (reg == 0x00) return last_rtc_sec;
        if (reg == 0x02) return last_rtc_min;
        if (reg == 0x04) return last_rtc_hour;
        return 0;
    }
    outb(0x70, reg);
    unsigned char val = inb(0x71);
    if (reg == 0x00) last_rtc_sec = val;
    else if (reg == 0x02) last_rtc_min = val;
    else if (reg == 0x04) last_rtc_hour = val;
    return val;
}

static int bcd_to_bin(unsigned char val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

void taskbar_draw_clock(void) {
    unsigned char raw_sec = read_rtc(0x00);
    unsigned char raw_min = read_rtc(0x02);
    unsigned char raw_hour = read_rtc(0x04);

    int sec = bcd_to_bin(raw_sec);
    int min = bcd_to_bin(raw_min);
    int hour = bcd_to_bin(raw_hour);

    char time_str[16];
    time_str[0] = (hour / 10) + '0';
    time_str[1] = (hour % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (min / 10) + '0';
    time_str[4] = (min % 10) + '0';
    time_str[5] = ':';
    time_str[6] = (sec / 10) + '0';
    time_str[7] = (sec % 10) + '0';
    time_str[8] = '\0';

    int clock_x = 940;
    int clock_y = TASKBAR_Y + 12;
    draw_rect(clock_x, clock_y - 2, 74, 16, 0xCE79);
    print_string(time_str, clock_x, clock_y, 0x0000);
}

void taskbar_draw_desktop_icons(void) {
    const char* icon_names[8] = {
        "Notepad", "Explorer", "Calc", "SysInfo", "Mem", "Pong", "Installer", "Readme"
    };
    const char* icon_files[8] = {
        "notepad.bin", "explorer.bin", "calc.bin", "sysinfo.bin", "mem.bin", "pong.bin", "installer.bin", "readme.txt"
    };
    const char* icon_badges[8] = {
        "NP", "EXP", "CAL", "CPU", "MEM", "PNG", "INS", "TXT"
    };
    unsigned short icon_colors[8] = {
        0x03EA, // Green
        0x24EE, // Cyan
        0xF621, // Amber
        0x0DE5, // Teal
        0x05E0, // Lime green for Mem
        0x7BEF, // Steel blue
        0x92E0, // Purple
        0xFFFF  // White
    };

    for (int i = 0; i < 8; i++) {
        int ix = 20;
        int iy = 16 + (i * 84);

        // Icon Graphic Box
        draw_3d_box(ix + 12, iy, 38, 38, 0, icon_colors[i]);
        print_string((char*)icon_badges[i], ix + 16, iy + 14, 0x0000);

        // Text label pill below icon
        int name_len = 0;
        while (icon_names[i][name_len] != '\0') name_len++;
        int label_w = (name_len * 9) + 8;
        int label_x = ix + 31 - (label_w / 2);
        if (label_x < 4) label_x = 4;

        draw_rect(label_x, iy + 42, label_w, 15, 0x0000);
        print_string((char*)icon_names[i], label_x + 4, iy + 44, 0xFFFF);

        // Subtitle badge
        const char* f = icon_files[i];
        if (f[name_len] != '\0') {
            draw_rect(label_x + (label_w/2) - 16, iy + 58, 32, 11, 0x2124);
            print_string(".bin", label_x + (label_w/2) - 14, iy + 59, 0x07E0);
        }
    }
}

void taskbar_draw(void) {
    // Taskbar base bar across bottom (y = 730 .. 767)
    draw_rect(0, TASKBAR_Y, 1024, 1, 0xFFFF);
    draw_rect(0, TASKBAR_Y + 1, 1024, 1, 0xCE79);
    draw_rect(0, TASKBAR_Y + 2, 1024, TASKBAR_HEIGHT - 2, 0xBDD7);

    // Start / maxOS button on left
    int btn_sunken = start_menu_open;
    unsigned short btn_fill = btn_sunken ? 0x9CD3 : 0xCE79;
    draw_3d_box(4, TASKBAR_Y + 4, 84, 30, btn_sunken, btn_fill);

    // Start logo (depressed offset)
    int s_off = btn_sunken ? 1 : 0;
    draw_rect(10 + s_off, TASKBAR_Y + 9 + s_off, 18, 18, 0x11EB);
    draw_rect(11 + s_off, TASKBAR_Y + 10 + s_off, 16, 16, 0x03EA);
    print_string("m", 15 + s_off, TASKBAR_Y + 14 + s_off, 0xFFFF);
    print_string("maxOS", 34 + s_off, TASKBAR_Y + 13 + s_off, 0x0000);

    // Running Applications Tabs (Multi-Instance)
    int run_count = maxp_get_instance_count();
    int active_id = maxp_get_active_instance_id();

    if (run_count == 0) {
        int tab_x = 96;
        int tab_w = 120;
        int tab_h = 28;
        int tab_y = TASKBAR_Y + 5;
        draw_3d_box(tab_x, tab_y, tab_w, tab_h, 1, 0xEF59);
        print_string("[D]", tab_x + 9, tab_y + 9, 0x0000);
        print_string("Desktop", tab_x + 39, tab_y + 9, 0x0000);
    } else {
        int max_w = 125;
        int tab_w = 640 / run_count;
        if (tab_w > max_w) tab_w = max_w;
        if (tab_w < 65) tab_w = 65;
        int tab_h = 28;
        int tab_y = TASKBAR_Y + 5;

        for (int i = 0; i < run_count; i++) {
            app_instance_t* inst = maxp_get_instance_by_index(i);
            if (!inst) continue;

            int tab_x = 96 + i * (tab_w + 4);
            if (tab_x + tab_w > 755) break;

            int is_tab_active = (inst->instance_id == active_id && !inst->is_minimized && !app_minimized);
            draw_3d_box(tab_x, tab_y, tab_w, tab_h, is_tab_active, is_tab_active ? 0xEF59 : 0xCE79);

            int tab_off = is_tab_active ? 1 : 0;
            char badge_buf[12];
            badge_buf[0] = '[';
            int bp = 1;
            for (int b = 0; inst->icon[b] != '\0' && bp < 9; b++) badge_buf[bp++] = inst->icon[b];
            badge_buf[bp++] = ']';
            badge_buf[bp] = '\0';
            print_string(badge_buf, tab_x + 6 + tab_off, tab_y + 8 + tab_off, inst->icon_color);

            if (tab_w >= 85) {
                print_string(inst->title, tab_x + 42 + tab_off, tab_y + 8 + tab_off, 0x0000);
            }
        }
    }

    // System Tray on right
    int tray_x = 760;
    int tray_y = TASKBAR_Y + 4;
    int tray_w = 258;
    int tray_h = 30;

    draw_3d_box(tray_x, tray_y, tray_w, tray_h, 1, 0xCE79);

    draw_rect(tray_x + 6, tray_y + 5, 36, 18, 0x0000);
    print_string("x64", tray_x + 10, tray_y + 9, 0x07E0);

    int disk_ok = maxfs_is_mounted();
    draw_rect(tray_x + 48, tray_y + 5, 38, 18, 0x0000);
    print_string("HDD", tray_x + 52, tray_y + 9, disk_ok ? 0x07E0 : 0x7BEF);

    draw_rect(tray_x + 92, tray_y + 5, 32, 18, 0x0000);
    print_string("R3", tray_x + 98, tray_y + 9, 0x07E0);

    taskbar_draw_clock();

    // Start Menu Popup
    if (start_menu_open) {
        int sm_x = 4;
        int sm_y = 415;
        int sm_w = 230;
        int sm_h = 312;

        draw_rect(sm_x, sm_y, sm_w, sm_h, 0x0000);
        draw_rect(sm_x + 1, sm_y + 1, sm_w - 2, 1, 0xFFFF);
        draw_rect(sm_x + 1, sm_y + 1, 1, sm_h - 2, 0xFFFF);
        draw_rect(sm_x + sm_w - 2, sm_y + 1, 1, sm_h - 2, 0x7BEF);
        draw_rect(sm_x + 1, sm_y + sm_h - 2, sm_w - 2, 1, 0x7BEF);
        draw_rect(sm_x + 2, sm_y + 2, sm_w - 4, sm_h - 4, 0xEF59);

        // Sidebar banner
        draw_rect(sm_x + 2, sm_y + 2, 24, sm_h - 4, 0x11EB);
        print_string("m", sm_x + 9, sm_y + sm_h - 60, 0xFFFF);
        print_string("a", sm_x + 9, sm_y + sm_h - 48, 0xFFFF);
        print_string("x", sm_x + 9, sm_y + sm_h - 36, 0xFFFF);
        print_string("O", sm_x + 9, sm_y + sm_h - 24, 0xFFFF);
        print_string("S", sm_x + 9, sm_y + sm_h - 12, 0xFFFF);

        // Header
        draw_rect(sm_x + 28, sm_y + 4, sm_w - 32, 22, 0x2417);
        print_string("maxOS v4.0 EventUpdate", sm_x + 34, sm_y + 8, 0xFFFF);

        // Menu Items
        const char* sm_items[7] = {
            "Notepad (new)",
            "Explorer (new)",
            "Calculator (new)",
            "SysInfo (new)",
            "Mem Monitor",
            "Pong Arcade (new)",
            "Installer"
        };
        const char* sm_badges[7] = { "NP", "EXP", "CAL", "CPU", "MEM", "PNG", "INS" };
        unsigned short sm_colors[7] = { 0x03EA, 0x24EE, 0xF621, 0x0DE5, 0x05E0, 0x7BEF, 0x92E0 };

        for (int i = 0; i < 7; i++) {
            int iy = sm_y + 30 + (i * 27);
            draw_rect(sm_x + 28, iy, 30, 22, sm_colors[i]);
            int badge_x = (i == 0) ? (sm_x + 34) : (sm_x + 30);
            print_string((char*)sm_badges[i], badge_x, iy + 6, 0x0000);
            print_string((char*)sm_items[i], sm_x + 64, iy + 6, 0x0000);
        }

        // Separator
        draw_rect(sm_x + 28, sm_y + 222, sm_w - 32, 1, 0x7BEF);
        draw_rect(sm_x + 28, sm_y + 223, sm_w - 32, 1, 0xFFFF);

        // System options
        draw_rect(sm_x + 28, sm_y + 230, 30, 22, 0xFCEF);
        print_string("TH", sm_x + 34, sm_y + 236, 0x0000);
        print_string("Themes (Key 1-9)", sm_x + 64, sm_y + 236, 0x0000);

        draw_rect(sm_x + 28, sm_y + 260, 30, 22, 0xF9A6);
        print_string("OFF", sm_x + 30, sm_y + 266, 0x0000);
        print_string("Shutdown System", sm_x + 64, sm_y + 266, 0x0000);
    }
}

int taskbar_handle_click(int mouse_x, int mouse_y) {
    // 1. Check Start button click
    if (mouse_x >= 4 && mouse_x <= 88 && mouse_y >= TASKBAR_Y + 4 && mouse_y <= TASKBAR_Y + 34) {
        prev_cursor();
        draw_3d_box(4, TASKBAR_Y + 4, 84, 30, 1, 0x9CD3);
        draw_rect(11, TASKBAR_Y + 10, 18, 18, 0x11EB);
        draw_rect(12, TASKBAR_Y + 11, 16, 16, 0x03EA);
        print_string("m", 16, TASKBAR_Y + 15, 0xFFFF);
        print_string("maxOS", 35, TASKBAR_Y + 14, 0x0000);
        draw_cursor(pos_x, pos_y);
        play_sound(750); sleep(35); no_sound();
        taskbar_toggle_start_menu();
        return 1;
    }

    // 2. Start Menu items
    if (start_menu_open) {
        int sm_x = 4;
        int sm_y = 415;
        int sm_w = 230;
        int sm_h = 312;

        if (mouse_x >= sm_x && mouse_x <= sm_x + sm_w && mouse_y >= sm_y && mouse_y <= sm_y + sm_h) {
            for (int i = 0; i < 7; i++) {
                int iy = sm_y + 30 + (i * 27);
                if (mouse_y >= iy && mouse_y <= iy + 25) {
                    prev_cursor();
                    draw_rect(sm_x + 28, iy, sm_w - 32, 24, 0x11EB);
                    draw_cursor(pos_x, pos_y);
                    play_sound(800); sleep(30); no_sound();
                    start_menu_open = 0;
                    app_minimized = 0;
                    if (i == 0) maxp_spawn_instance(MAXP_APP_NOTEPAD, 0, 0);
                    else if (i == 1) maxp_spawn_instance(MAXP_APP_EXPLORER, 0, 0);
                    else if (i == 2) maxp_spawn_instance(MAXP_APP_CALC, 0, 0);
                    else if (i == 3) maxp_spawn_instance(MAXP_APP_SYSINFO, 0, 0);
                    else if (i == 4) maxp_spawn_instance(MAXP_APP_MEM, 0, 0);
                    else if (i == 5) maxp_spawn_instance(MAXP_APP_PONG, 0, 0);
                    else if (i == 6) maxp_spawn_instance(MAXP_APP_INSTALLER, 0, 0);
                    return 1;
                }
            }

            if (mouse_y >= sm_y + 228 && mouse_y <= sm_y + 252) {
                theme = (theme % 9) + 1;
                start_menu_open = 0;
                draw_window();
                draw_cursor(pos_x, pos_y);
                play_sound(700); sleep(30); no_sound();
                return 1;
            }

            if (mouse_y >= sm_y + 258 && mouse_y <= sm_y + 286) {
                start_menu_open = 0;
                shutdown();
                return 1;
            }
            return 1;
        } else {
            start_menu_open = 0;
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
    }

    // 3. Taskbar Tabs click (Multi-Instance)
    int run_count = maxp_get_instance_count();
    int active_id = maxp_get_active_instance_id();

    if (run_count == 0) {
        int tab_x = 96;
        int tab_w = 120;
        if (mouse_x >= tab_x && mouse_x <= tab_x + tab_w && mouse_y >= TASKBAR_Y + 5 && mouse_y <= TASKBAR_Y + 33) {
            return 1;
        }
    } else {
        int max_w = 125;
        int tab_w = 640 / run_count;
        if (tab_w > max_w) tab_w = max_w;
        if (tab_w < 65) tab_w = 65;
        int tab_h = 28;
        int tab_y = TASKBAR_Y + 5;

        for (int i = 0; i < run_count; i++) {
            app_instance_t* inst = maxp_get_instance_by_index(i);
            if (!inst) continue;

            int tab_x = 96 + i * (tab_w + 4);
            if (tab_x + tab_w > 755) break;

            if (mouse_x >= tab_x && mouse_x <= tab_x + tab_w && mouse_y >= TASKBAR_Y + 5 && mouse_y <= TASKBAR_Y + 33) {
                prev_cursor();
                draw_3d_box(tab_x, tab_y, tab_w, tab_h, 1, 0x9CD3);
                char badge_buf[12];
                badge_buf[0] = '[';
                int bp = 1;
                for (int b = 0; inst->icon[b] != '\0' && bp < 9; b++) badge_buf[bp++] = inst->icon[b];
                badge_buf[bp++] = ']';
                badge_buf[bp] = '\0';
                print_string(badge_buf, tab_x + 7, tab_y + 9, inst->icon_color);
                if (tab_w >= 85) print_string(inst->title, tab_x + 43, tab_y + 9, 0x0000);
                draw_cursor(pos_x, pos_y);
                play_sound(750); sleep(35); no_sound();

                if (inst->instance_id == active_id && !inst->is_minimized) {
                    app_minimized = 1;
                    inst->is_minimized = 1;
                } else {
                    maxp_set_active_instance(inst->instance_id);
                    app_minimized = 0;
                    inst->is_minimized = 0;
                }
                draw_window();
                draw_cursor(pos_x, pos_y);
                return 1;
            }
        }
    }

    // 4. Desktop Icons click (spawns new instances!)
    for (int i = 0; i < 8; i++) {
        int ix = 20;
        int iy = 16 + (i * 84);
        if (mouse_x >= ix && mouse_x <= ix + 64 && mouse_y >= iy && mouse_y <= iy + 76) {
            app_minimized = 0;
            if (i == 0) maxp_spawn_instance(MAXP_APP_NOTEPAD, 0, 0);
            else if (i == 1) maxp_spawn_instance(MAXP_APP_EXPLORER, 0, 0);
            else if (i == 2) maxp_spawn_instance(MAXP_APP_CALC, 0, 0);
            else if (i == 3) maxp_spawn_instance(MAXP_APP_SYSINFO, 0, 0);
            else if (i == 4) maxp_spawn_instance(MAXP_APP_MEM, 0, 0);
            else if (i == 5) maxp_spawn_instance(MAXP_APP_PONG, 0, 0);
            else if (i == 6) maxp_spawn_instance(MAXP_APP_INSTALLER, 0, 0);
            else if (i == 7) {
                maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad (Readme)", "readme.txt");
            }
            return 1;
        }
    }

    // 5. Swallow any clicks on taskbar
    if (mouse_y >= TASKBAR_Y) {
        return 1;
    }

    return 0;
}

int taskbar_handle_key(char ascii_char, unsigned char scan_code) {
    if (scan_code == 0x5B || scan_code == 0x5C) {
        taskbar_toggle_start_menu();
        return 1;
    }

    if (scan_code == 0x44) {
        app_instance_t* inst = maxp_get_active_instance();
        if (inst) {
            app_minimized = !app_minimized;
            inst->is_minimized = app_minimized;
            play_sound(650); sleep(20); no_sound();
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
    }

    if (start_menu_open) {
        if (scan_code == 0x01 || ascii_char == 'c' || ascii_char == 'C') {
            taskbar_close_start_menu();
            return 1;
        }
        if (ascii_char == '1') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_NOTEPAD, 0, 0); return 1; }
        if (ascii_char == '2') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_EXPLORER, 0, 0); return 1; }
        if (ascii_char == '3') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_CALC, 0, 0); return 1; }
        if (ascii_char == '4') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_SYSINFO, 0, 0); return 1; }
        if (ascii_char == '5') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_MEM, 0, 0); return 1; }
        if (ascii_char == '6') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_PONG, 0, 0); return 1; }
        if (ascii_char == '7') { start_menu_open = 0; maxp_spawn_instance(MAXP_APP_INSTALLER, 0, 0); return 1; }
        if (ascii_char == 't' || ascii_char == 'T') {
            theme = (theme % 9) + 1;
            start_menu_open = 0;
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
        if (ascii_char == 'x' || ascii_char == 'X') {
            start_menu_open = 0;
            shutdown();
            return 1;
        }
    }

    return 0;
}

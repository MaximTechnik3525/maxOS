#include "taskbar.h"
#include "maxp.h"
#include "maxfs.h"
#include "notepad.h"
#include "explorer.h"
#include "installer.h"
#include "calc.h"
#include "sysinfo.h"
#include "pong.h"

extern unsigned short* _gfx_memory_backend;
#define gfx_memory _gfx_memory_backend

extern int pos_x, pos_y;
extern int drag;
extern int theme;

void print_string(char* str, int x, int y, unsigned short color);
void draw_cursor(int mouse_x, int mouse_y);
void prev_cursor(void);
void draw_window(void);
void play_sound(unsigned int nfreq);
void sleep(unsigned int ms);
void no_sound(void);
void int_str(int num, char* str);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void shutdown(void);

int start_menu_open = 0;
static int app_minimized = 0;

static void draw_rect(int rx, int ry, int rw, int rh, unsigned short color) {
    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= 768) continue;
        int row = y * 1024;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= 1024) continue;
            gfx_memory[row + x] = color;
        }
    }
}

static void draw_3d_box(int bx, int by, int bw, int bh, int sunken, unsigned short fill) {
    draw_rect(bx, by, bw, bh, 0x0000);
    if (!sunken) {
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
    } else {
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0x7BEF);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0x7BEF);
        draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0xFFFF);
        draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0xFFFF);
    }
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
}

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

static unsigned char read_rtc(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
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
    // Clear clock area
    draw_rect(clock_x, clock_y - 2, 74, 16, 0xCE79);
    print_string(time_str, clock_x, clock_y, 0x0000);
}

void taskbar_draw_desktop_icons(void) {
    // 7 desktop icons on the left side
    const char* icon_names[7] = {
        "Notepad", "Explorer", "Calc", "SysInfo", "Pong", "Installer", "Readme"
    };
    const char* icon_files[7] = {
        "notepad.maxP", "explorer.maxP", "calc.maxP", "sysinfo.maxP", "pong.maxP", "install.maxP", "readme.txt"
    };
    const char* icon_badges[7] = {
        "NP", "EXP", "CAL", "CPU", "PNG", "INS", "TXT"
    };
    unsigned short icon_colors[7] = {
        0x03EA, // Green
        0x24EE, // Cyan
        0xF621, // Amber
        0x0DE5, // Teal
        0x7BEF, // Steel blue
        0x92E0, // Purple
        0xFFFF  // White
    };

    for (int i = 0; i < 7; i++) {
        int ix = 20;
        int iy = 20 + (i * 86);

        // Icon Graphic Box (36x36 3D beveled)
        draw_3d_box(ix + 12, iy, 38, 38, 0, icon_colors[i]);
        
        // Inner badge label
        print_string((char*)icon_badges[i], ix + 16, iy + 14, 0x0000);

        // Text label pill below icon with dark background for high contrast
        int name_len = 0;
        while (icon_names[i][name_len] != '\0') name_len++;
        int label_w = (name_len * 9) + 8;
        int label_x = ix + 31 - (label_w / 2);
        if (label_x < 4) label_x = 4;

        draw_rect(label_x, iy + 42, label_w, 15, 0x0000); // Black label pill
        print_string((char*)icon_names[i], label_x + 4, iy + 44, 0xFFFF);

        // Small extension subtitle
        const char* f = icon_files[i];
        if (f[name_len] != '\0') {
            // maxP subtitle
            draw_rect(label_x + (label_w/2) - 16, iy + 58, 32, 11, 0x2124);
            print_string(".maxP", label_x + (label_w/2) - 15, iy + 59, 0x07E0);
        }
    }
}

void taskbar_draw(void) {
    // 1. Taskbar base bar across bottom (y = 730 .. 767)
    draw_rect(0, TASKBAR_Y, 1024, 1, 0xFFFF);      // Top 3D highlight
    draw_rect(0, TASKBAR_Y + 1, 1024, 1, 0xCE79);  // Top secondary bevel
    draw_rect(0, TASKBAR_Y + 2, 1024, TASKBAR_HEIGHT - 2, 0xBDD7); // Classic gray taskbar

    // 2. Start / maxOS button on left (x = 4, y = 734, w = 82, h = 30)
    int btn_sunken = start_menu_open;
    unsigned short btn_fill = btn_sunken ? 0x9CD3 : 0xCE79;
    draw_3d_box(4, TASKBAR_Y + 4, 84, 30, btn_sunken, btn_fill);

    // Start logo square
    draw_rect(10, TASKBAR_Y + 9, 18, 18, 0x11EB); // Navy/Blue logo
    draw_rect(11, TASKBAR_Y + 10, 16, 16, 0x03EA); // Green center
    print_string("m", 15, TASKBAR_Y + 14, 0xFFFF);
    print_string("maxOS", 34, TASKBAR_Y + 13, 0x0000);

    // 3. Active Window Tab in taskbar
    int active_app = maxp_get_active_app();
    int tab_x = 96;
    int tab_w = 170;
    int tab_h = 28;
    int tab_y = TASKBAR_Y + 5;

    const char* tab_title = "Desktop";
    const char* tab_icon = "[D]";
    unsigned short tab_col = 0x0000;

    if (active_app == MAXP_APP_NOTEPAD) {
        tab_title = "Notepad.maxP";
        tab_icon = "[NP]";
        tab_col = 0x03EA;
    } else if (active_app == MAXP_APP_EXPLORER) {
        tab_title = "Explorer.maxP";
        tab_icon = "[EXP]";
        tab_col = 0x24EE;
    } else if (active_app == MAXP_APP_CALC) {
        tab_title = "Calc.maxP";
        tab_icon = "[CAL]";
        tab_col = 0xF621;
    } else if (active_app == MAXP_APP_SYSINFO) {
        tab_title = "SysInfo.maxP";
        tab_icon = "[CPU]";
        tab_col = 0x0DE5;
    } else if (active_app == MAXP_APP_PONG) {
        tab_title = "Pong.maxP";
        tab_icon = "[PNG]";
        tab_col = 0x7BEF;
    } else if (active_app == MAXP_APP_INSTALLER) {
        tab_title = "Install.maxP";
        tab_icon = "[INS]";
        tab_col = 0x92E0;
    }

    int is_tab_active = (active_app != MAXP_APP_NONE && !app_minimized);
    draw_3d_box(tab_x, tab_y, tab_w, tab_h, is_tab_active, is_tab_active ? 0xEF59 : 0xCE79);
    print_string((char*)tab_icon, tab_x + 8, tab_y + 8, tab_col);
    print_string((char*)tab_title, tab_x + 48, tab_y + 8, 0x0000);

    // 4. System Tray on right (x = 810 .. 1018, y = 734, w = 208, h = 30)
    int tray_x = 810;
    int tray_y = TASKBAR_Y + 4;
    int tray_w = 208;
    int tray_h = 30;

    draw_3d_box(tray_x, tray_y, tray_w, tray_h, 1, 0xCE79);

    // [x64] Badge
    draw_rect(tray_x + 6, tray_y + 5, 36, 18, 0x0000);
    print_string("x64", tray_x + 10, tray_y + 9, 0x07E0); // Bright green

    // [HDD] Badge
    int disk_ok = maxfs_is_mounted();
    draw_rect(tray_x + 48, tray_y + 5, 38, 18, 0x0000);
    print_string("HDD", tray_x + 52, tray_y + 9, disk_ok ? 0x07E0 : 0x7BEF);

    // Real-Time Clock
    taskbar_draw_clock();

    // 5. Start Menu Popup if open
    if (start_menu_open) {
        int sm_x = 4;
        int sm_y = 445;
        int sm_w = 230;
        int sm_h = 282;

        // Popup frame with 3D shadow
        draw_rect(sm_x, sm_y, sm_w, sm_h, 0x0000);
        draw_rect(sm_x + 1, sm_y + 1, sm_w - 2, 1, 0xFFFF);
        draw_rect(sm_x + 1, sm_y + 1, 1, sm_h - 2, 0xFFFF);
        draw_rect(sm_x + sm_w - 2, sm_y + 1, 1, sm_h - 2, 0x7BEF);
        draw_rect(sm_x + 1, sm_y + sm_h - 2, sm_w - 2, 1, 0x7BEF);
        draw_rect(sm_x + 2, sm_y + 2, sm_w - 4, sm_h - 4, 0xEF59);

        // Sidebar banner (Windows / classic style gradient)
        draw_rect(sm_x + 2, sm_y + 2, 24, sm_h - 4, 0x11EB);
        // Vertical text "maxOS"
        print_string("m", sm_x + 9, sm_y + sm_h - 60, 0xFFFF);
        print_string("a", sm_x + 9, sm_y + sm_h - 48, 0xFFFF);
        print_string("x", sm_x + 9, sm_y + sm_h - 36, 0xFFFF);
        print_string("O", sm_x + 9, sm_y + sm_h - 24, 0xFFFF);
        print_string("S", sm_x + 9, sm_y + sm_h - 12, 0xFFFF);

        // Top Header
        draw_rect(sm_x + 28, sm_y + 4, sm_w - 32, 22, 0x2417);
        print_string("maxOS x86_64", sm_x + 34, sm_y + 8, 0xFFFF);

        // Program items
        const char* sm_items[6] = {
            "Notepad.maxP",
            "Explorer.maxP",
            "Calculator.maxP",
            "SysInfo.maxP",
            "Pong Arcade.maxP",
            "Installer.maxP"
        };
        const char* sm_badges[6] = { "NP", "EXP", "CAL", "CPU", "PNG", "INS" };
        unsigned short sm_colors[6] = { 0x03EA, 0x24EE, 0xF621, 0x0DE5, 0x7BEF, 0x92E0 };

        for (int i = 0; i < 6; i++) {
            int iy = sm_y + 32 + (i * 28);
            draw_rect(sm_x + 28, iy, 30, 22, sm_colors[i]);
            int badge_x = (i == 0) ? (sm_x + 34) : (sm_x + 30);
            print_string((char*)sm_badges[i], badge_x, iy + 6, 0x0000);
            print_string((char*)sm_items[i], sm_x + 64, iy + 6, 0x0000);
        }

        // Separator line
        draw_rect(sm_x + 28, sm_y + 204, sm_w - 32, 1, 0x7BEF);
        draw_rect(sm_x + 28, sm_y + 205, sm_w - 32, 1, 0xFFFF);

        // System Options
        draw_rect(sm_x + 28, sm_y + 212, 30, 22, 0xFCEF);
        print_string("TH", sm_x + 34, sm_y + 218, 0x0000);
        print_string("Themes (Key 1-9)", sm_x + 64, sm_y + 218, 0x0000);

        draw_rect(sm_x + 28, sm_y + 242, 30, 22, 0xF9A6);
        print_string("OFF", sm_x + 30, sm_y + 248, 0x0000);
        print_string("Shutdown System", sm_x + 64, sm_y + 248, 0x0000);
    }
}

int taskbar_handle_click(int mouse_x, int mouse_y) {
    // 1. Check Start button click (x = 4..88, y = 734..764)
    if (mouse_x >= 4 && mouse_x <= 88 && mouse_y >= TASKBAR_Y + 4 && mouse_y <= TASKBAR_Y + 34) {
        taskbar_toggle_start_menu();
        return 1;
    }

    // 2. If Start Menu is open, check menu items
    if (start_menu_open) {
        int sm_x = 4;
        int sm_y = 445;
        int sm_w = 230;
        int sm_h = 282;

        if (mouse_x >= sm_x && mouse_x <= sm_x + sm_w && mouse_y >= sm_y && mouse_y <= sm_y + sm_h) {
            // Check 6 program items
            for (int i = 0; i < 6; i++) {
                int iy = sm_y + 32 + (i * 28);
                if (mouse_y >= iy && mouse_y <= iy + 26) {
                    start_menu_open = 0;
                    app_minimized = 0;
                    if (i == 0) maxp_launch_app(MAXP_APP_NOTEPAD);
                    else if (i == 1) maxp_launch_app(MAXP_APP_EXPLORER);
                    else if (i == 2) maxp_launch_app(MAXP_APP_CALC);
                    else if (i == 3) maxp_launch_app(MAXP_APP_SYSINFO);
                    else if (i == 4) maxp_launch_app(MAXP_APP_PONG);
                    else if (i == 5) maxp_launch_app(MAXP_APP_INSTALLER);
                    return 1;
                }
            }

            // Check Themes item
            if (mouse_y >= sm_y + 212 && mouse_y <= sm_y + 236) {
                theme = (theme % 9) + 1;
                start_menu_open = 0;
                draw_window();
                draw_cursor(pos_x, pos_y);
                play_sound(700); sleep(30); no_sound();
                return 1;
            }

            // Check Shutdown item
            if (mouse_y >= sm_y + 242 && mouse_y <= sm_y + 270) {
                start_menu_open = 0;
                shutdown();
                return 1;
            }
            return 1;
        } else {
            // Click outside start menu closes it
            start_menu_open = 0;
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
    }

    // 3. Check Taskbar active window tab click (x = 96 .. 266, y = 735 .. 763)
    int tab_x = 96;
    int tab_w = 170;
    if (mouse_x >= tab_x && mouse_x <= tab_x + tab_w && mouse_y >= TASKBAR_Y + 5 && mouse_y <= TASKBAR_Y + 33) {
        int active_app = maxp_get_active_app();
        if (active_app != MAXP_APP_NONE) {
            app_minimized = !app_minimized;
            play_sound(650); sleep(20); no_sound();
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
    }

    // 4. Check Desktop Icons clicks on left (when not covered by active window or if clicked directly)
    for (int i = 0; i < 7; i++) {
        int ix = 20;
        int iy = 20 + (i * 86);
        if (mouse_x >= ix && mouse_x <= ix + 64 && mouse_y >= iy && mouse_y <= iy + 76) {
            app_minimized = 0;
            if (i == 0) maxp_launch_app(MAXP_APP_NOTEPAD);
            else if (i == 1) maxp_launch_app(MAXP_APP_EXPLORER);
            else if (i == 2) maxp_launch_app(MAXP_APP_CALC);
            else if (i == 3) maxp_launch_app(MAXP_APP_SYSINFO);
            else if (i == 4) maxp_launch_app(MAXP_APP_PONG);
            else if (i == 5) maxp_launch_app(MAXP_APP_INSTALLER);
            else if (i == 6) {
                // Open readme.txt in Notepad
                notepad_open_file_by_name("readme.txt");
                maxp_set_active_app(MAXP_APP_NOTEPAD);
            }
            return 1;
        }
    }

    // 5. Swallow any clicks inside taskbar area
    if (mouse_y >= TASKBAR_Y) {
        return 1;
    }

    return 0;
}

int taskbar_handle_key(char ascii_char, unsigned char scan_code) {
    // Windows key or 'm' / 'M' toggles start menu
    if (scan_code == 0x5B || scan_code == 0x5C) {
        taskbar_toggle_start_menu();
        return 1;
    }

    // F10 toggles minimize / restore active window
    if (scan_code == 0x44) {
        int active_app = maxp_get_active_app();
        if (active_app != MAXP_APP_NONE) {
            app_minimized = !app_minimized;
            play_sound(650); sleep(20); no_sound();
            draw_window();
            draw_cursor(pos_x, pos_y);
            return 1;
        }
    }

    if (start_menu_open) {
        if (scan_code == 0x01 || ascii_char == 'c' || ascii_char == 'C') { // Esc
            taskbar_close_start_menu();
            return 1;
        }
        if (ascii_char == '1') { start_menu_open = 0; maxp_launch_app(MAXP_APP_NOTEPAD); return 1; }
        if (ascii_char == '2') { start_menu_open = 0; maxp_launch_app(MAXP_APP_EXPLORER); return 1; }
        if (ascii_char == '3') { start_menu_open = 0; maxp_launch_app(MAXP_APP_CALC); return 1; }
        if (ascii_char == '4') { start_menu_open = 0; maxp_launch_app(MAXP_APP_SYSINFO); return 1; }
        if (ascii_char == '5') { start_menu_open = 0; maxp_launch_app(MAXP_APP_PONG); return 1; }
        if (ascii_char == '6') { start_menu_open = 0; maxp_launch_app(MAXP_APP_INSTALLER); return 1; }
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

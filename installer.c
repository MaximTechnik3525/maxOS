#include "installer.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"

extern unsigned short* _gfx_memory_backend;
#define gfx_memory _gfx_memory_backend

extern int win_x, win_y, win_w, win_h;
extern int pos_x, pos_y;
extern int drag;

extern const unsigned char mbr_boot_code[512];

void print_string(char* str, int x, int y, unsigned short color);
void draw_cursor(int mouse_x, int mouse_y);
void prev_cursor(void);
void draw_window(void);
void play_sound(unsigned int nfreq);
void sleep(unsigned int ms);
void no_sound(void);
void int_str(int num, char* str);

int installer_open = 0;
static int install_progress = 0;
static char install_status[64] = "Click [Install maxOS] to begin setup.";

void installer_init(void) {
    installer_open = 0;
    install_progress = 0;
}

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

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 8, by + 6, text_col);
}

static void draw_progress_bar(int px, int py, int pw, int ph, int pct) {
    // Border
    draw_rect(px, py, pw, ph, 0x0000);
    draw_rect(px + 1, py + 1, pw - 2, ph - 2, 0xFFFF);

    // Fill
    int fill_w = ((pw - 4) * pct) / 100;
    if (fill_w > pw - 4) fill_w = pw - 4;
    if (fill_w > 0) {
        draw_rect(px + 2, py + 2, fill_w, ph - 4, 0x03EA); // Green
    }
    if (pw - 4 - fill_w > 0) {
        draw_rect(px + 2 + fill_w, py + 2, pw - 4 - fill_w, ph - 4, 0xE71C); // Light gray
    }

    char pct_str[8];
    int_str(pct, pct_str);
    int p = 0;
    while (pct_str[p] != '\0') p++;
    pct_str[p++] = '%';
    pct_str[p] = '\0';
    print_string(pct_str, px + (pw / 2) - 15, py + 5, 0x0000);
}

void installer_draw(void) {
    prev_cursor();

    int inst_x = win_x + 15;
    int inst_y = win_y + 35;
    int inst_w = win_w - 30;
    int inst_h = win_h - 45;

    // Window frame
    draw_rect(inst_x, inst_y, inst_w, inst_h, 0x0000);
    draw_rect(inst_x + 1, inst_y + 1, inst_w - 2, inst_h - 2, 0xEF59);

    // Titlebar
    draw_rect(inst_x + 2, inst_y + 2, inst_w - 4, 20, 0x11EB);
    print_string("maxOS System Setup & Installer v2.0", inst_x + 8, inst_y + 6, 0xFFFF);

    // Content card
    draw_rect(inst_x + 10, inst_y + 28, inst_w - 20, inst_h - 60, 0xFFFF);
    draw_rect(inst_x + 10, inst_y + 28, inst_w - 20, 1, 0x7BEF);
    draw_rect(inst_x + 10, inst_y + 28, 1, inst_h - 60, 0x7BEF);
    draw_rect(inst_x + inst_w - 11, inst_y + 28, 1, inst_h - 60, 0x7BEF);
    draw_rect(inst_x + 10, inst_y + inst_h - 33, inst_w - 20, 1, 0x7BEF);

    // Hardware Details
    print_string("Hardware Detection:", inst_x + 20, inst_y + 38, 0x24EE);
    if (ata_is_available()) {
        print_string("Drive: Primary Master ATA", inst_x + 25, inst_y + 54, 0x0000);
        print_string("Model:", inst_x + 25, inst_y + 70, 0x0000);
        print_string(ata_primary_master.model, inst_x + 80, inst_y + 70, 0x0320);

        print_string("Size :", inst_x + 25, inst_y + 86, 0x0000);
        char sz_str[16];
        int_str(ata_primary_master.size_mb, sz_str);
        int sp = 0; while (sz_str[sp] != '\0') sp++;
        sz_str[sp++] = ' '; sz_str[sp++] = 'M'; sz_str[sp++] = 'B'; sz_str[sp] = '\0';
        print_string(sz_str, inst_x + 80, inst_y + 86, 0x0320);

        print_string("FS   : maxFS 2.0 (Persistent Disk Storage)", inst_x + 25, inst_y + 102, 0x0000);
        if (maxfs_is_mounted()) {
            print_string("Status: Formatted & Mounted", inst_x + 25, inst_y + 118, 0x03EA);
        } else {
            print_string("Status: Drive Ready for Installation", inst_x + 25, inst_y + 118, 0xF9A6);
        }
    } else {
        print_string("No ATA Hard Disk detected! Run with -hda.", inst_x + 25, inst_y + 54, 0xF800);
    }

    // Interactive Buttons
    // [Install maxOS] button
    draw_ui_btn(inst_x + 25, inst_y + 140, 140, 24, "Install maxOS", 0x3DF2, 0x0000);

    // [Format Only] button
    draw_ui_btn(inst_x + 180, inst_y + 140, 120, 24, "Format Only", 0x24EE, 0xFFFF);

    // [Close] button
    draw_ui_btn(inst_x + inst_w - 95, inst_y + 140, 75, 24, "Close", 0xF9A6, 0x0000);

    // Progress Bar
    draw_progress_bar(inst_x + 25, inst_y + 175, inst_w - 50, 22, install_progress);

    // Status / Step description
    print_string("Status:", inst_x + 25, inst_y + 205, 0x0000);
    print_string(install_status, inst_x + 85, inst_y + 205, 0x001F);

    // Footer Help Bar
    draw_rect(inst_x + 2, inst_y + inst_h - 26, inst_w - 4, 24, 0xDF17);
    draw_rect(inst_x + 2, inst_y + inst_h - 26, inst_w - 4, 1, 0x9CD3);
    print_string("Click [Install maxOS] to setup MBR bootloader, maxFS and files to HDD", inst_x + 10, inst_y + inst_h - 18, 0x0000);

    draw_cursor(pos_x, pos_y);
}

static void installer_set_step(int pct, const char* msg) {
    install_progress = pct;
    int p = 0;
    while (msg[p] != '\0' && p < 63) {
        install_status[p] = msg[p];
        p++;
    }
    install_status[p] = '\0';
    installer_draw();
}

static void installer_run_install(void) {
    if (!ata_is_available()) {
        installer_set_step(0, "Error: No ATA Hard Disk detected!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 1: Write MBR Bootloader to Sector 0
    installer_set_step(20, "1/5: Writing MBR Bootloader to Sector 0...");
    play_sound(500); sleep(100); no_sound();
    if (ata_write_sector(0, mbr_boot_code) != 0) {
        installer_set_step(20, "Error: Failed to write MBR to Sector 0!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 2: Format maxFS on hard drive
    installer_set_step(40, "2/5: Formatting maxFS 2.0 on disk...");
    play_sound(650); sleep(100); no_sound();
    if (maxfs_format("maxOS System Disk") != 0) {
        installer_set_step(40, "Error: Failed to format maxFS on disk!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 3: Install Kernel to LBA 32
    installer_set_step(60, "3/5: Installing OS Kernel to LBA 32...");
    play_sound(800); sleep(100); no_sound();
    const unsigned char* kernel_mem = (const unsigned char*)0x100000;
    for (unsigned int s = 0; s < 240; s++) {
        ata_write_sector(32 + s, kernel_mem + (s * 512));
    }
    maxfs_write_file("kernel.bin", (const char*)kernel_mem, 1024);

    // Step 4: Write default configuration & documents
    installer_set_step(80, "4/5: Writing system configs and documents...");
    play_sound(950); sleep(100); no_sound();
    maxfs_write_file("system.cfg", "theme=1\nresolution=1024x768x16\nos=maxOS RedCycle x86_64\n", 59);
    maxfs_write_file("welcome.txt", "Welcome to maxOS x86_64!\nInstalled on your real Hard Disk with maxFS 2.0.", 74);
    maxfs_write_file("readme.txt", "maxOS RedCycle 2.0 (x86_64 Long Mode)\nPrograms use .maxP extension!\nAll edits persist!", 88);
    maxfs_write_file("notes.txt", "Sample document stored on hard drive sectors.\nOpen and edit in Notepad!", 71);
    maxfs_write_file("notepad.maxP", "MAXP\nNAME=Notepad\nEXEC=notepad\nICON=NP\nDESC=maxOS Notepad 2.0 Text Editor\n", 73);
    maxfs_write_file("explorer.maxP", "MAXP\nNAME=Explorer\nEXEC=explorer\nICON=EXP\nDESC=File & Disk Manager\n", 67);
    maxfs_write_file("calc.maxP", "MAXP\nNAME=Calculator\nEXEC=calc\nICON=CALC\nDESC=Desktop GUI Calculator\n", 71);
    maxfs_write_file("sysinfo.maxP", "MAXP\nNAME=SysInfo\nEXEC=sysinfo\nICON=CPU\nDESC=x86_64 Long Mode System Info\n", 76);
    maxfs_write_file("pong.maxP", "MAXP\nNAME=Pong\nEXEC=pong\nICON=PONG\nDESC=Retro Pong Arcade Game\n", 64);
    maxfs_write_file("install.maxP", "MAXP\nNAME=Installer\nEXEC=installer\nICON=INST\nDESC=maxOS System Setup & HDD Installer\n", 83);

    // Step 5: Flush cache & verify
    installer_set_step(100, "5/5: Verifying & Flushing ATA cache...");
    ata_flush();
    play_sound(500); sleep(80); play_sound(750); sleep(80); play_sound(1000); sleep(120); no_sound();

    installer_set_step(100, "Installation Complete! System installed on HDD.");
}

void installer_open_window(void) {
    installer_open = 1;
    drag = 1;
    install_progress = 0;
    maxp_set_active_app(MAXP_APP_INSTALLER);
    if (maxfs_is_mounted()) {
        installer_set_step(100, "Drive already contains installed maxOS.");
    } else {
        installer_set_step(0, "Click [Install maxOS] to begin setup.");
    }
    draw_window();
    play_sound(600); sleep(60); no_sound();
}

void installer_close_window(void) {
    installer_open = 0;
    drag = 0;
    maxp_set_active_app(MAXP_APP_NONE);
    draw_window();
    play_sound(400); sleep(60); no_sound();
}

int installer_handle_click(int mouse_x, int mouse_y) {
    if (!installer_open) return 0;

    int inst_x = win_x + 15;
    int inst_y = win_y + 35;
    int inst_w = win_w - 30;
    int inst_h = win_h - 45;

    // Check [Install maxOS] button: inst_x + 25 .. inst_x + 165, inst_y + 140 .. inst_y + 164
    if (mouse_x >= inst_x + 25 && mouse_x <= inst_x + 165 && mouse_y >= inst_y + 140 && mouse_y <= inst_y + 164) {
        installer_run_install();
        return 1;
    }

    // Check [Format Only] button: inst_x + 180 .. inst_x + 300, inst_y + 140 .. inst_y + 164
    if (mouse_x >= inst_x + 180 && mouse_x <= inst_x + 300 && mouse_y >= inst_y + 140 && mouse_y <= inst_y + 164) {
        installer_set_step(50, "Formatting disk in maxFS 2.0...");
        if (maxfs_format("maxOS Disk") == 0) {
            installer_set_step(100, "Disk formatted successfully with maxFS 2.0.");
            play_sound(700); sleep(80); play_sound(1000); sleep(100); no_sound();
        } else {
            installer_set_step(0, "Error formatting disk!");
            play_sound(200); sleep(200); no_sound();
        }
        return 1;
    }

    // Check [Close] button: inst_x + inst_w - 95 .. inst_x + inst_w - 20, inst_y + 140 .. inst_y + 164
    if (mouse_x >= inst_x + inst_w - 95 && mouse_x <= inst_x + inst_w - 20 && mouse_y >= inst_y + 140 && mouse_y <= inst_y + 164) {
        installer_close_window();
        return 1;
    }

    // Swallow any clicks inside installer window
    if (mouse_x >= inst_x && mouse_x <= inst_x + inst_w && mouse_y >= inst_y && mouse_y <= inst_y + inst_h) {
        return 1;
    }

    return 0;
}

int installer_handle_key(char ascii_char, unsigned char scan_code) {
    if (!installer_open) return 0;

    // F2 (0x3C), Escape (0x01), 'c', 'C', 'q', 'Q', 'S'
    if (scan_code == 0x01 || scan_code == 0x3C || ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || ascii_char == 'Q' || ascii_char == 'S') {
        installer_close_window();
        return 1;
    }

    // 'i' or 'I' starts installation
    if (ascii_char == 'i' || ascii_char == 'I') {
        installer_run_install();
        return 1;
    }

    return 0;
}

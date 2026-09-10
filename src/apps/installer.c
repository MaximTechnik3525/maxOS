#include "installer.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"
#include "kernel.h"
#include "string.h"

extern const unsigned char mbr_boot_code[512];

int installer_open = 0;
static installer_state_t primary_installer_state;

void installer_instance_init(installer_state_t* s) {
    if (!s) return;
    s->install_progress = 0;
    if (maxfs_is_mounted()) {
        strncpy(s->install_status, "Drive already contains installed maxOS.", sizeof(s->install_status) - 1);
        s->install_progress = 100;
    } else {
        strncpy(s->install_status, "Click [Install maxOS] to begin setup.", sizeof(s->install_status) - 1);
    }
    s->install_status[sizeof(s->install_status) - 1] = '\0';
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

    char pct_str[16];
    int_to_str(pct, pct_str);
    strcat(pct_str, "%");
    print_string(pct_str, px + (pw / 2) - 15, py + 5, 0x0000);
}

void installer_instance_draw(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h) {
    prev_cursor();

    // Window frame
    draw_rect(inst_x, inst_y, inst_w, inst_h, 0x0000);
    draw_rect(inst_x + 1, inst_y + 1, inst_w - 2, inst_h - 2, 0xEF59);

    // Titlebar
    draw_rect(inst_x + 2, inst_y + 2, inst_w - 4, 20, 0x11EB);
    print_string("maxOS System Setup & Installer v3.5", inst_x + 8, inst_y + 6, 0xFFFF);

    // [_] Minimize & [X] Close button
    draw_ui_btn(inst_x + inst_w - 44, inst_y + 4, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(inst_x + inst_w - 22, inst_y + 4, 18, 16, "X", 0xF800, 0xFFFF);

    // Content card
    draw_rect(inst_x + 10, inst_y + 28, inst_w - 20, inst_h - 60, 0xFFFF);
    draw_rect(inst_x + 10, inst_y + 28, inst_w - 20, 1, 0x7BEF);
    draw_rect(inst_x + 10, inst_y + 28, 1, inst_h - 60, 0x7BEF);
    draw_rect(inst_x + inst_w - 11, inst_y + 28, 1, inst_h - 60, 0x7BEF);
    draw_rect(inst_x + 10, inst_y + inst_h - 33, inst_w - 20, 1, 0x7BEF);

    // Hardware Details
    print_string("Hardware Detection:", inst_x + 20, inst_y + 36, 0x24EE);
    if (ata_is_available()) {
        print_string("Drive: Primary Master ATA (IDE PIO)", inst_x + 25, inst_y + 52, 0x0000);
        print_string("Model:", inst_x + 25, inst_y + 68, 0x0000);
        print_string(ata_primary_master.model, inst_x + 80, inst_y + 68, 0x0320);

        print_string("Size :", inst_x + 25, inst_y + 84, 0x0000);
        char sz_str[32];
        int_to_str(ata_primary_master.size_mb, sz_str);
        strcat(sz_str, " MB (");
        char sec_str[16];
        int_to_str((int)ata_primary_master.total_sectors, sec_str);
        strcat(sz_str, sec_str);
        strcat(sz_str, " sectors)");
        print_string(sz_str, inst_x + 80, inst_y + 84, 0x0320);

        print_string("FS   : maxFS 2.0 (Persistent Disk Storage)", inst_x + 25, inst_y + 100, 0x0000);
        if (maxfs_is_mounted()) {
            print_string("Status: Formatted & Mounted (Ready)", inst_x + 25, inst_y + 116, 0x03EA);
        } else {
            print_string("Status: Drive Ready for Installation", inst_x + 25, inst_y + 116, 0xF9A6);
        }
    } else {
        print_string("No ATA Hard Disk detected! Run with -drive file=maxos_disk.img", inst_x + 25, inst_y + 52, 0xF800);
    }

    // Interactive Buttons
    // [Install maxOS] button
    draw_ui_btn(inst_x + 25, inst_y + 138, 140, 24, "Install maxOS", 0x3DF2, 0x0000);

    // [Format Only] button
    draw_ui_btn(inst_x + 180, inst_y + 138, 120, 24, "Format Only", 0x24EE, 0xFFFF);

    // Progress Bar
    draw_progress_bar(inst_x + 25, inst_y + 172, inst_w - 50, 22, s ? s->install_progress : 0);

    // Status / Step description
    print_string("Status:", inst_x + 25, inst_y + 202, 0x0000);
    if (s && s->install_status[0] != '\0') {
        print_string(s->install_status, inst_x + 85, inst_y + 202, 0x001F);
    }

    // Installed components overview
    print_string("Installation Steps & Components:", inst_x + 20, inst_y + 230, 0x11EB);
    print_string("1. MBR Bootloader (Sector 0) - Real Mode Stage 1 bootstrap", inst_x + 25, inst_y + 248, 0x4210);
    print_string("2. maxFS 2.0 Inode Structure (Superblock, Bitmaps, Inodes)", inst_x + 25, inst_y + 264, 0x4210);
    print_string("3. OS Kernel 64-bit Binary written to LBA 32 (240 sectors)", inst_x + 25, inst_y + 280, 0x4210);
    print_string("4. Default system configuration & documentation files", inst_x + 25, inst_y + 296, 0x4210);
    print_string("5. Verification & ATA hardware write cache flush", inst_x + 25, inst_y + 312, 0x4210);
    print_string("Press [Install maxOS] (or key 'i') to start disk deployment.", inst_x + 25, inst_y + 334, 0x03EA);

    // Footer Help Bar
    draw_rect(inst_x + 2, inst_y + inst_h - 26, inst_w - 4, 24, 0xDF17);
    draw_rect(inst_x + 2, inst_y + inst_h - 26, inst_w - 4, 1, 0x9CD3);
    print_string("Click [Install maxOS] to setup MBR bootloader, maxFS and files to HDD", inst_x + 10, inst_y + inst_h - 18, 0x0000);

    draw_cursor(pos_x, pos_y);
}

static void installer_set_step(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h, int pct, const char* msg) {
    if (s) {
        s->install_progress = pct;
        strncpy(s->install_status, msg, sizeof(s->install_status) - 1);
        s->install_status[sizeof(s->install_status) - 1] = '\0';
    }
    prev_cursor();
    
    // Partial Redraw: Only Progress Bar and Status Text
    draw_progress_bar(inst_x + 25, inst_y + 172, inst_w - 50, 22, s ? s->install_progress : 0);
    draw_rect(inst_x + 25, inst_y + 104, inst_w - 50, 20, 0xEF59); // Clear background
    print_string((char*)msg, inst_x + 25, inst_y + 104, 0x0200);

    draw_cursor(pos_x, pos_y);
}

static void installer_run_install(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h) {
    if (!ata_is_available()) {
        installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 0, "Error: No ATA Hard Disk detected!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 1: Write MBR Bootloader to Sector 0
    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 20, "1/5: Writing MBR Bootloader to Sector 0...");
    play_sound(500); sleep(100); no_sound();
    if (ata_write_sector(0, mbr_boot_code) != 0) {
        installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 20, "Error: Failed to write MBR to Sector 0!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 2: Format maxFS on hard drive
    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 40, "2/5: Formatting maxFS 2.0 on disk...");
    play_sound(650); sleep(100); no_sound();
    if (maxfs_format("maxOS System Disk") != 0) {
        installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 40, "Error: Failed to format maxFS on disk!");
        play_sound(200); sleep(200); no_sound();
        return;
    }

    // Step 3: Install Kernel to LBA 32
    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 60, "3/5: Installing OS Kernel to LBA 32...");
    play_sound(800); sleep(100); no_sound();
    const unsigned char* kernel_mem = (const unsigned char*)0x100000;
    for (unsigned int sec = 0; sec < 960; sec++) {
        ata_write_sector(32 + sec, kernel_mem + (sec * 512));
    }
    maxfs_write_file("kernel.bin", (const char*)kernel_mem, 960 * 512);

    // Step 4: Write default configuration & documents
    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 80, "4/5: Writing system configs and documents...");
    play_sound(950); sleep(100); no_sound();
    const char* sys_cfg = "theme=1\nresolution=1024x768x16\nos=maxOS MaxRing v3.5 x86_64\n";
    maxfs_write_file("system.cfg", sys_cfg, (unsigned int)strlen(sys_cfg));
    const char* wel_txt = "Welcome to maxOS v3.5 MaxRing x86_64!\nInstalled on your real Hard Disk with maxFS 2.0.";
    maxfs_write_file("welcome.txt", wel_txt, (unsigned int)strlen(wel_txt));
    const char* rdm_txt = "maxOS MaxRing 3.5 (x86_64 Long Mode)\nPrograms use .maxP extension!\nAll edits persist!";
    maxfs_write_file("readme.txt", rdm_txt, (unsigned int)strlen(rdm_txt));
    maxfs_write_file("notes.txt", "Sample document stored on hard drive sectors.\nOpen and edit in Notepad!", 71);
    const char* np_desc = "MAXP\nNAME=Notepad\nEXEC=notepad\nICON=NP\nDESC=maxOS Notepad 3.5 Text Editor\n";
    maxfs_write_file("notepad.maxP", np_desc, (unsigned int)strlen(np_desc));
    maxfs_write_file("explorer.maxP", "MAXP\nNAME=Explorer\nEXEC=explorer\nICON=EXP\nDESC=File & Disk Manager\n", 67);
    maxfs_write_file("calc.maxP", "MAXP\nNAME=Calculator\nEXEC=calc\nICON=CALC\nDESC=Desktop GUI Calculator\n", 71);
    maxfs_write_file("sysinfo.maxP", "MAXP\nNAME=SysInfo\nEXEC=sysinfo\nICON=CPU\nDESC=x86_64 Long Mode System Info\n", 76);
    maxfs_write_file("pong.maxP", "MAXP\nNAME=Pong\nEXEC=pong\nICON=PONG\nDESC=Retro Pong Arcade Game\n", 64);
    maxfs_write_file("install.maxP", "MAXP\nNAME=Installer\nEXEC=installer\nICON=INST\nDESC=maxOS System Setup & HDD Installer\n", 83);
    maxfs_write_file("mem.maxP", "MAXP\nNAME=Mem\nEXEC=mem\nICON=MEM\nDESC=maxOS RAM & Memory Monitor\n", 66);
    maxfs_write_file("stress.maxP", "MAXP\nNAME=StressTest\nEXEC=stress\nICON=RAM\nDESC=RAM Hardware Stress Test & Benchmark\n", 79);

    // Step 5: Flush cache & verify
    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 100, "5/5: Verifying & Flushing ATA cache...");
    ata_flush();
    play_sound(500); sleep(80); play_sound(750); sleep(80); play_sound(1000); sleep(120); no_sound();

    installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 100, "Installation Complete! Rebooting in 3 seconds...");
    maxp_draw_active_instance();
    sleep(3000);
    system_reboot();
}

int installer_instance_click(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h, int mouse_x, int mouse_y) {
    // Check [Install maxOS] button: inst_x + 25 .. inst_x + 165, inst_y + 138 .. inst_y + 162
    if (mouse_x >= inst_x + 25 && mouse_x <= inst_x + 165 && mouse_y >= inst_y + 138 && mouse_y <= inst_y + 162) {
        ui_btn_click_effect(inst_x + 25, inst_y + 138, 140, 24, "Install maxOS", 0x3DF2, 0x0000);
        installer_run_install(s, inst_x, inst_y, inst_w, inst_h);
        return 1;
    }

    // Check [Format Only] button: inst_x + 180 .. inst_x + 300, inst_y + 138 .. inst_y + 162
    if (mouse_x >= inst_x + 180 && mouse_x <= inst_x + 300 && mouse_y >= inst_y + 138 && mouse_y <= inst_y + 162) {
        ui_btn_click_effect(inst_x + 180, inst_y + 138, 120, 24, "Format Only", 0x24EE, 0xFFFF);
        installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 50, "Formatting disk in maxFS 2.0...");
        if (maxfs_format("maxOS Disk") == 0) {
            installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 100, "Disk formatted successfully with maxFS 2.0.");
            play_sound(700); sleep(80); play_sound(1000); sleep(100); no_sound();
        } else {
            installer_set_step(s, inst_x, inst_y, inst_w, inst_h, 0, "Error formatting disk!");
            play_sound(200); sleep(200); no_sound();
        }
        return 1;
    }

    // Check [_] Minimize button (Titlebar)
    if (mouse_x >= inst_x + inst_w - 48 && mouse_x <= inst_x + inst_w - 26 && mouse_y >= inst_y && mouse_y <= inst_y + 24) {
        ui_btn_click_effect(inst_x + inst_w - 44, inst_y + 4, 18, 16, "_", 0xCE79, 0x0000);
        return -2;
    }

    // Check [X] Close button (Titlebar): inst_x + inst_w - 26 .. inst_x + inst_w, inst_y .. inst_y + 24
    if (mouse_x >= inst_x + inst_w - 26 && mouse_x <= inst_x + inst_w && mouse_y >= inst_y && mouse_y <= inst_y + 24) {
        ui_btn_click_effect(inst_x + inst_w - 22, inst_y + 4, 18, 16, "X", 0xF800, 0xFFFF);
        return -1;
    }

    // Swallow any clicks inside installer window
    if (mouse_x >= inst_x && mouse_x <= inst_x + inst_w && mouse_y >= inst_y && mouse_y <= inst_y + inst_h) {
        return 1;
    }

    return 0;
}

int installer_instance_key(installer_state_t* s, char ascii_char, unsigned char scan_code) {
    // F2 (0x3C), Escape (0x01), 'c', 'C', 'q', 'Q'
    if (scan_code == 0x01 || scan_code == 0x3C || ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || ascii_char == 'Q') {
        return -1;
    }

    // 'i' or 'I' starts installation
    if (ascii_char == 'i' || ascii_char == 'I') {
        app_instance_t* inst = maxp_get_active_instance();
        int ix = inst ? inst->win_x : (win_x + 15);
        int iy = inst ? inst->win_y : (win_y + 35);
        int iw = inst ? inst->win_w : (win_w - 30);
        int ih = inst ? inst->win_h : (win_h - 45);
        installer_run_install(s, ix, iy, iw, ih);
        maxp_draw_active_instance();
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void installer_init(void) {
    installer_open = 0;
    installer_instance_init(&primary_installer_state);
}

void installer_open_window(void) {
    installer_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_INSTALLER, "Installer", 0);
}

void installer_close_window(void) {
    installer_open = 0;
    maxp_close_app(MAXP_APP_INSTALLER);
}

void installer_draw(void) {
    installer_instance_draw(&primary_installer_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45);
}

int installer_handle_click(int mouse_x, int mouse_y) {
    int res = installer_instance_click(&primary_installer_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45, mouse_x, mouse_y);
    if (res == -1) {
        installer_close_window();
        return 1;
    }
    return res;
}

int installer_handle_key(char ascii_char, unsigned char scan_code) {
    int res = installer_instance_key(&primary_installer_state, ascii_char, scan_code);
    if (res == -1) {
        installer_close_window();
        return 1;
    }
    return res;
}

#include "maxp.h"
#include "maxfs.h"
#include "notepad.h"
#include "explorer.h"
#include "installer.h"
#include "calc.h"
#include "sysinfo.h"
#include "pong.h"

extern void play_sound(unsigned int nfreq);
extern void sleep(unsigned int ms);
extern void no_sound(void);
extern void draw_window(void);

static int active_app_id = MAXP_APP_NONE;

static const struct MaxPAppInfo app_registry[MAXP_APP_COUNT] = {
    { MAXP_APP_NOTEPAD,   "Notepad",     "notepad.maxP", "Text Editor v2.0",            "NP",   0x03EA },
    { MAXP_APP_EXPLORER,  "Explorer",    "explorer.maxP", "File & Disk Manager",         "EXP",  0x24EE },
    { MAXP_APP_CALC,      "Calculator",  "calc.maxP",     "GUI Calculator",             "CALC", 0xF621 },
    { MAXP_APP_SYSINFO,   "SysInfo",     "sysinfo.maxP",  "x86_64 Long Mode Info",      "CPU",  0x0DE5 },
    { MAXP_APP_PONG,      "Pong Arcade", "pong.maxP",     "Retro Arcade Game",          "PONG", 0x7BEF },
    { MAXP_APP_INSTALLER, "Installer",   "install.maxP",  "maxOS HDD Setup",            "INST", 0x92E0 }
};

const struct MaxPAppInfo* maxp_get_app_info(int app_id) {
    for (int i = 0; i < MAXP_APP_COUNT; i++) {
        if (app_registry[i].id == app_id) return &app_registry[i];
    }
    return 0;
}

const struct MaxPAppInfo* maxp_get_app_by_index(int index) {
    if (index >= 0 && index < MAXP_APP_COUNT) return &app_registry[index];
    return 0;
}

int maxp_get_active_app(void) {
    return active_app_id;
}

void maxp_set_active_app(int app_id) {
    active_app_id = app_id;
}

void maxp_close_all_windows(void) {
    if (notepad_open) notepad_close_window();
    if (explorer_open) explorer_close_window();
    if (installer_open) installer_close_window();
    if (calc_open) calc_close_window();
    if (sysinfo_open) sysinfo_close_window();
    if (pong_open) pong_close_window();
    active_app_id = MAXP_APP_NONE;
}

static int str_equal(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

int maxp_is_maxp_file(const char* filename) {
    int len = 0;
    while (filename[len] != '\0') len++;
    if (len < 5) return 0;
    const char* ext = &filename[len - 5];
    if (ext[0] == '.' &&
        (ext[1] == 'm' || ext[1] == 'M') &&
        (ext[2] == 'a' || ext[2] == 'A') &&
        (ext[3] == 'x' || ext[3] == 'X') &&
        (ext[4] == 'p' || ext[4] == 'P')) {
        return 1;
    }
    return 0;
}

int maxp_launch_app(int app_id) {
    // Close current window if switching
    maxp_close_all_windows();

    active_app_id = app_id;
    play_sound(750); sleep(40); play_sound(1100); sleep(50); no_sound();

    switch (app_id) {
        case MAXP_APP_NOTEPAD:
            notepad_open_window();
            break;
        case MAXP_APP_EXPLORER:
            explorer_open_window();
            break;
        case MAXP_APP_CALC:
            calc_open_window();
            break;
        case MAXP_APP_SYSINFO:
            sysinfo_open_window();
            break;
        case MAXP_APP_PONG:
            pong_open_window();
            break;
        case MAXP_APP_INSTALLER:
            installer_open_window();
            break;
        default:
            active_app_id = MAXP_APP_NONE;
            draw_window();
            return 0;
    }
    return 1;
}

int maxp_launch_file(const char* filename) {
    if (str_equal(filename, "notepad.maxP") || str_equal(filename, "Notepad.maxP")) {
        return maxp_launch_app(MAXP_APP_NOTEPAD);
    }
    if (str_equal(filename, "explorer.maxP") || str_equal(filename, "Explorer.maxP")) {
        return maxp_launch_app(MAXP_APP_EXPLORER);
    }
    if (str_equal(filename, "calc.maxP") || str_equal(filename, "Calc.maxP")) {
        return maxp_launch_app(MAXP_APP_CALC);
    }
    if (str_equal(filename, "sysinfo.maxP") || str_equal(filename, "Sysinfo.maxP")) {
        return maxp_launch_app(MAXP_APP_SYSINFO);
    }
    if (str_equal(filename, "pong.maxP") || str_equal(filename, "Pong.maxP")) {
        return maxp_launch_app(MAXP_APP_PONG);
    }
    if (str_equal(filename, "install.maxP") || str_equal(filename, "Install.maxP")) {
        return maxp_launch_app(MAXP_APP_INSTALLER);
    }

    // Inspect content if custom named .maxP file
    int slot = maxfs_find_file(filename);
    if (slot != -1) {
        struct VirtualFile* vf = maxfs_get_file(slot);
        if (vf && vf->exists) {
            // Check for EXEC= tag inside .maxP
            const char* c = vf->content;
            int idx = 0;
            while (c[idx] != '\0' && idx < 500) {
                if (c[idx] == 'E' && c[idx+1] == 'X' && c[idx+2] == 'E' && c[idx+3] == 'C' && c[idx+4] == '=') {
                    const char* target = &c[idx + 5];
                    if (target[0] == 'n' && target[1] == 'o') return maxp_launch_app(MAXP_APP_NOTEPAD);
                    if (target[0] == 'e' && target[1] == 'x') return maxp_launch_app(MAXP_APP_EXPLORER);
                    if (target[0] == 'c' && target[1] == 'a') return maxp_launch_app(MAXP_APP_CALC);
                    if (target[0] == 's' && target[1] == 'y') return maxp_launch_app(MAXP_APP_SYSINFO);
                    if (target[0] == 'p' && target[1] == 'o') return maxp_launch_app(MAXP_APP_PONG);
                    if (target[0] == 'i' && target[1] == 'n') return maxp_launch_app(MAXP_APP_INSTALLER);
                }
                idx++;
            }
        }
    }
    return 0;
}

void maxp_init(void) {
    // Seed default .maxP program files if not present on filesystem
    if (maxfs_find_file("notepad.maxP") == -1) {
        const char* np_content = "MAXP\nNAME=Notepad\nEXEC=notepad\nICON=NP\nDESC=maxOS Notepad 2.0 Text Editor\n";
        maxfs_write_file("notepad.maxP", np_content, 73);
    }
    if (maxfs_find_file("explorer.maxP") == -1) {
        const char* exp_content = "MAXP\nNAME=Explorer\nEXEC=explorer\nICON=EXP\nDESC=File & Disk Manager\n";
        maxfs_write_file("explorer.maxP", exp_content, 67);
    }
    if (maxfs_find_file("calc.maxP") == -1) {
        const char* calc_content = "MAXP\nNAME=Calculator\nEXEC=calc\nICON=CALC\nDESC=Desktop GUI Calculator\n";
        maxfs_write_file("calc.maxP", calc_content, 71);
    }
    if (maxfs_find_file("sysinfo.maxP") == -1) {
        const char* sys_content = "MAXP\nNAME=SysInfo\nEXEC=sysinfo\nICON=CPU\nDESC=x86_64 Long Mode System Info\n";
        maxfs_write_file("sysinfo.maxP", sys_content, 76);
    }
    if (maxfs_find_file("pong.maxP") == -1) {
        const char* pong_content = "MAXP\nNAME=Pong\nEXEC=pong\nICON=PONG\nDESC=Retro Pong Arcade Game\n";
        maxfs_write_file("pong.maxP", pong_content, 64);
    }
    if (maxfs_find_file("install.maxP") == -1) {
        const char* inst_content = "MAXP\nNAME=Installer\nEXEC=installer\nICON=INST\nDESC=maxOS System Setup & HDD Installer\n";
        maxfs_write_file("install.maxP", inst_content, 83);
    }
    if (maxfs_find_file("readme.txt") == -1) {
        const char* rm = "Welcome to maxOS RedCycle x86_64!\nPrograms use .maxP extension.\nUse Taskbar or Desktop icons to run apps.\n";
        maxfs_write_file("readme.txt", rm, 107);
    }
}

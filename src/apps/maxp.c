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
#include "taskbar.h"
#include "task.h"
#include "debug.h"

static int active_app_id = MAXP_APP_NONE;

static const struct MaxPAppInfo app_registry[MAXP_APP_COUNT] = {
    { MAXP_APP_NOTEPAD,   "Notepad",     "notepad.maxP", "Text Editor v3.1",            "NP",   0x03EA },
    { MAXP_APP_EXPLORER,  "Explorer",    "explorer.maxP", "File & Disk Manager",         "EXP",  0x24EE },
    { MAXP_APP_CALC,      "Calculator",  "calc.maxP",     "GUI Calculator",             "CALC", 0xF621 },
    { MAXP_APP_SYSINFO,   "SysInfo",     "sysinfo.maxP",  "x86_64 Long Mode Info",      "CPU",  0x0DE5 },
    { MAXP_APP_PONG,      "Pong Arcade", "pong.maxP",     "Retro Arcade Game",          "PONG", 0x7BEF },
    { MAXP_APP_INSTALLER, "Installer",   "install.maxP",  "maxOS HDD Setup",            "INST", 0x92E0 },
    { MAXP_APP_MEM,       "Mem",         "mem.maxP",      "RAM & Memory Usage Monitor", "MEM",  0x24EE }
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
    if (active_app_id != app_id) {
        if (active_app_id != MAXP_APP_NONE && app_id == MAXP_APP_NONE) {
            const struct MaxPAppInfo* prev_info = maxp_get_app_info(active_app_id);
            debug_log_app_event(prev_info ? prev_info->name : "App", "Window Closed", active_app_id);
        }
        active_app_id = app_id;
    }
}

int maxp_is_app_running(int app_id) {
    switch (app_id) {
        case MAXP_APP_NOTEPAD: return notepad_open;
        case MAXP_APP_EXPLORER: return explorer_open;
        case MAXP_APP_CALC: return calc_open;
        case MAXP_APP_SYSINFO: return sysinfo_open;
        case MAXP_APP_PONG: return pong_open;
        case MAXP_APP_INSTALLER: return installer_open;
        case MAXP_APP_MEM: return mem_open;
        default: return 0;
    }
}

int maxp_get_running_apps(int* out_apps, int max_count) {
    int count = 0;
    for (int i = 1; i <= MAXP_APP_COUNT && count < max_count; i++) {
        if (maxp_is_app_running(i)) {
            out_apps[count++] = i;
        }
    }
    return count;
}

int maxp_get_running_count(void) {
    int count = 0;
    for (int i = 1; i <= MAXP_APP_COUNT; i++) {
        if (maxp_is_app_running(i)) count++;
    }
    return count;
}

void maxp_close_app(int app_id) {
    const struct MaxPAppInfo* info = maxp_get_app_info(app_id);
    if (info) {
        debug_log_app_event(info->name, "Closing App Window", app_id);
    }
    switch (app_id) {
        case MAXP_APP_NOTEPAD: notepad_open = 0; break;
        case MAXP_APP_EXPLORER: explorer_open = 0; break;
        case MAXP_APP_CALC: calc_open = 0; break;
        case MAXP_APP_SYSINFO: sysinfo_open = 0; break;
        case MAXP_APP_PONG: pong_open = 0; break;
        case MAXP_APP_INSTALLER: installer_open = 0; break;
        case MAXP_APP_MEM: mem_open = 0; break;
    }
    drag = 0;
    for (int i = 1; i < MAX_TASKS; i++) {
        task_t* t = task_get_by_pid(i);
        if (t && t->app_id == app_id && t->state != TASK_UNUSED && t->state != TASK_DEAD) {
            task_kill(i);
        }
    }
    int next_app = MAXP_APP_NONE;
    for (int i = 1; i <= MAXP_APP_COUNT; i++) {
        if (maxp_is_app_running(i)) {
            next_app = i;
            break;
        }
    }
    active_app_id = next_app;
    taskbar_set_app_minimized(0);
    draw_window();
}

void maxp_close_all_windows(void) {
    if (active_app_id != MAXP_APP_NONE) {
        const struct MaxPAppInfo* info = maxp_get_app_info(active_app_id);
        debug_log_app_event(info ? info->name : "App", "Closing All Windows", active_app_id);
    }
    if (notepad_open) notepad_close_window();
    if (explorer_open) explorer_close_window();
    if (installer_open) installer_close_window();
    if (calc_open) calc_close_window();
    if (sysinfo_open) sysinfo_close_window();
    if (pong_open) pong_close_window();
    if (mem_open) mem_close_window();
    for (int i = 1; i < MAX_TASKS; i++) {
        task_t* t = task_get_by_pid(i);
        if (t && t->app_id != 0 && t->state != TASK_UNUSED && t->state != TASK_DEAD) {
            task_kill(i);
        }
    }
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
    const struct MaxPAppInfo* info = maxp_get_app_info(app_id);
    debug_log_app_event(info ? info->name : "App", "Launching Application", app_id);

    // If already running, bring to focus
    if (maxp_is_app_running(app_id)) {
        active_app_id = app_id;
        draw_window();
        return 1;
    }

    // Register a process in the preemptive scheduler
    task_create(info ? info->name : "App", 0, 1, app_id);

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
        case MAXP_APP_MEM:
            mem_open_window();
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
    if (str_equal(filename, "mem.maxP") || str_equal(filename, "Mem.maxP")) {
        return maxp_launch_app(MAXP_APP_MEM);
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
                    if (target[0] == 'm' && target[1] == 'e') return maxp_launch_app(MAXP_APP_MEM);
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
        const char* np_content = "MAXP\nNAME=Notepad\nEXEC=notepad\nICON=NP\nDESC=maxOS Notepad 3.1 Text Editor\n";
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
    if (maxfs_find_file("mem.maxP") == -1) {
        const char* mem_content = "MAXP\nNAME=Mem\nEXEC=mem\nICON=MEM\nDESC=maxOS RAM & Memory Monitor\n";
        maxfs_write_file("mem.maxP", mem_content, 66);
    }
    if (maxfs_find_file("readme.txt") == -1) {
        const char* rm = "Welcome to maxOS RedCycle v3.1 x86_64!\nPrograms use .maxP extension.\nUse Taskbar or Desktop icons to run apps.\n";
        maxfs_write_file("readme.txt", rm, 112);
    }
}

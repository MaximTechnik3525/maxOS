#include "maxp.h"
#include "maxb.h"
#include "app_binaries.h"
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
static int active_instance_id = 0;
static app_instance_t instances[MAX_APP_INSTANCES];
static unsigned char instance_state_pool[MAX_APP_INSTANCES][4096];

static const struct MaxPAppInfo app_registry[MAXP_APP_COUNT] = {
    { MAXP_APP_NOTEPAD,   "Notepad",     "notepad.maxP", "Text Editor v3.2",            "NP",   0x03EA },
    { MAXP_APP_EXPLORER,  "Explorer",    "explorer.maxP", "File & Disk Manager",         "EXP",  0x24EE },
    { MAXP_APP_CALC,      "Calculator",  "calc.maxP",     "GUI Calculator",             "CALC", 0xF621 },
    { MAXP_APP_SYSINFO,   "SysInfo",     "sysinfo.maxP",  "x86_64 Long Mode Info",      "CPU",  0x0DE5 },
    { MAXP_APP_PONG,      "Pong Arcade", "pong.maxP",     "Retro Arcade Game",          "PONG", 0x7BEF },
    { MAXP_APP_INSTALLER, "Installer",   "install.maxP",  "maxOS HDD Setup",            "INST", 0x92E0 },
    { MAXP_APP_MEM,       "Mem",         "mem.maxP",      "RAM & Memory Usage Monitor", "MEM",  0x05E0 }
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

static inline int str_equal(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

static inline void str_copy_limit(char* dest, const char* src, int max_len) {
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}

int maxp_is_maxp_file(const char* filename) {
    int len = (int)strlen(filename);
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

int maxp_get_instance_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0) count++;
    }
    return count;
}

app_instance_t* maxp_get_instance(int instance_id) {
    if (instance_id <= 0) return 0;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id == instance_id) return &instances[i];
    }
    return 0;
}

app_instance_t* maxp_get_instance_by_index(int index) {
    int cur = 0;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0) {
            if (cur == index) return &instances[i];
            cur++;
        }
    }
    return 0;
}

app_instance_t* maxp_get_active_instance(void) {
    return maxp_get_instance(active_instance_id);
}

int maxp_get_active_instance_id(void) {
    return active_instance_id;
}

void maxp_set_active_instance(int instance_id) {
    app_instance_t* inst = maxp_get_instance(instance_id);
    if (inst) {
        active_instance_id = instance_id;
        active_app_id = inst->app_type;
        taskbar_set_app_minimized(0);
    } else {
        active_instance_id = 0;
        active_app_id = MAXP_APP_NONE;
    }
}

void maxp_cycle_active_instance(void) {
    int count = maxp_get_instance_count();
    if (count <= 1) return;

    int cur_idx = -1;
    for (int i = 0; i < count; i++) {
        app_instance_t* inst = maxp_get_instance_by_index(i);
        if (inst && inst->instance_id == active_instance_id) {
            cur_idx = i;
            break;
        }
    }

    int next_idx = (cur_idx + 1) % count;
    app_instance_t* next_inst = maxp_get_instance_by_index(next_idx);
    if (next_inst) {
        maxp_set_active_instance(next_inst->instance_id);
    }
}

int maxp_get_active_app(void) {
    app_instance_t* inst = maxp_get_active_instance();
    return inst ? inst->app_type : active_app_id;
}

void maxp_set_active_app(int app_id) {
    active_app_id = app_id;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].app_type == app_id) {
            active_instance_id = instances[i].instance_id;
            taskbar_set_app_minimized(0);
            return;
        }
    }
}

int maxp_is_app_running(int app_id) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].app_type == app_id) return 1;
    }
    return 0;
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
    return maxp_get_instance_count();
}

int maxp_spawn_instance(int app_type, const char* custom_title, const char* file_arg) {
    int slot = -1;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id == 0) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        debug_log_app_event("MaxP", "Max instances reached (16)", app_type);
        return 0;
    }

    int type_count = 0;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].app_type == app_type) {
            type_count++;
        }
    }

    static int id_seq = 1;
    int inst_id = id_seq++;
    if (id_seq > 10000) id_seq = 1;

    app_instance_t* inst = &instances[slot];
    inst->instance_id = inst_id;
    inst->app_type = app_type;
    inst->is_minimized = 0;
    inst->state = (void*)instance_state_pool[slot];
    inst->state_size = 4096;
    for (int b = 0; b < 4096; b++) instance_state_pool[slot][b] = 0;

    int stagger = (slot * 25) % 150;

    // Default title formatting
    char title_buf[32];
    if (custom_title && custom_title[0] != '\0') {
        str_copy_limit(title_buf, custom_title, 32);
    } else {
        const char* base_name = "App";
        if (app_type == MAXP_APP_NOTEPAD) base_name = "Notepad";
        else if (app_type == MAXP_APP_CALC) base_name = "Calc";
        else if (app_type == MAXP_APP_PONG) base_name = "Pong";
        else if (app_type == MAXP_APP_EXPLORER) base_name = "Explorer";
        else if (app_type == MAXP_APP_SYSINFO) base_name = "SysInfo";
        else if (app_type == MAXP_APP_MEM) base_name = "Mem";
        else if (app_type == MAXP_APP_STRESS) base_name = "Stress";
        else if (app_type == MAXP_APP_INSTALLER) base_name = "Install";

        int p = 0;
        while (base_name[p] != '\0') { title_buf[p] = base_name[p]; p++; }
        if (type_count > 0) {
            title_buf[p++] = ' ';
            title_buf[p++] = '#';
            title_buf[p++] = (char)('1' + type_count);
        }
        title_buf[p] = '\0';
    }
    str_copy_limit(inst->title, title_buf, 32);

    inst->tick = 0;
    inst->close = 0;

    switch (app_type) {
        case MAXP_APP_NOTEPAD:
            inst->win_x = 90 + stagger; inst->win_y = 45 + stagger;
            inst->win_w = 780; inst->win_h = 520;
            str_copy_limit(inst->icon, "NP", 8);
            inst->icon_color = 0x03EA;
            inst->draw = (void (*)(void*, int, int, int, int))notepad_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))notepad_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))notepad_instance_key;
            notepad_instance_init((notepad_state_t*)inst->state, file_arg);
            notepad_open = 1;
            break;

        case MAXP_APP_CALC:
            inst->win_x = 180 + stagger; inst->win_y = 60 + stagger;
            inst->win_w = 340; inst->win_h = 380;
            str_copy_limit(inst->icon, "CAL", 8);
            inst->icon_color = 0xF621;
            inst->draw = (void (*)(void*, int, int, int, int))calc_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))calc_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))calc_instance_key;
            calc_instance_init((calc_state_t*)inst->state);
            calc_open = 1;
            break;

        case MAXP_APP_PONG:
            inst->win_x = 160 + stagger; inst->win_y = 55 + stagger;
            inst->win_w = 520; inst->win_h = 380;
            str_copy_limit(inst->icon, "PNG", 8);
            inst->icon_color = 0x7BEF;
            inst->draw = (void (*)(void*, int, int, int, int))pong_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))pong_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))pong_instance_key;
            inst->tick = (void (*)(void*, int, int, int, int))pong_instance_tick;
            pong_instance_init((pong_state_t*)inst->state, inst->win_x, inst->win_y, inst->win_w, inst->win_h);
            pong_open = 1;
            break;

        case MAXP_APP_EXPLORER:
            inst->win_x = 70 + stagger; inst->win_y = 45 + stagger;
            inst->win_w = 780; inst->win_h = 520;
            str_copy_limit(inst->icon, "EXP", 8);
            inst->icon_color = 0x24EE;
            inst->draw = (void (*)(void*, int, int, int, int))explorer_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))explorer_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))explorer_instance_key;
            explorer_instance_init((explorer_state_t*)inst->state);
            explorer_open = 1;
            break;

        case MAXP_APP_SYSINFO:
            inst->win_x = 140 + stagger; inst->win_y = 55 + stagger;
            inst->win_w = 540; inst->win_h = 380;
            str_copy_limit(inst->icon, "CPU", 8);
            inst->icon_color = 0x0DE5;
            inst->draw = (void (*)(void*, int, int, int, int))sysinfo_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))sysinfo_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))sysinfo_instance_key;
            sysinfo_open = 1;
            break;

        case MAXP_APP_MEM:
            inst->win_x = 80 + stagger; inst->win_y = 45 + stagger;
            inst->win_w = 760; inst->win_h = 490;
            str_copy_limit(inst->icon, "MEM", 8);
            inst->icon_color = 0x05E0;
            inst->draw = (void (*)(void*, int, int, int, int))mem_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))mem_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))mem_instance_key;
            inst->tick = (void (*)(void*, int, int, int, int))mem_instance_tick;
            mem_instance_init((mem_state_t*)inst->state, 0);
            mem_open = 1;
            break;

        case MAXP_APP_STRESS:
            inst->win_x = 80 + stagger; inst->win_y = 45 + stagger;
            inst->win_w = 760; inst->win_h = 490;
            str_copy_limit(inst->icon, "RAM", 8);
            inst->icon_color = 0xF800;
            inst->draw = (void (*)(void*, int, int, int, int))mem_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))mem_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))mem_instance_key;
            inst->tick = (void (*)(void*, int, int, int, int))mem_instance_tick;
            mem_instance_init((mem_state_t*)inst->state, 2);
            mem_open = 1;
            break;

        case MAXP_APP_INSTALLER:
            inst->win_x = 120 + stagger; inst->win_y = 60 + stagger;
            inst->win_w = 640; inst->win_h = 440;
            str_copy_limit(inst->icon, "INS", 8);
            inst->icon_color = 0x92E0;
            inst->draw = (void (*)(void*, int, int, int, int))installer_instance_draw;
            inst->handle_click = (int (*)(void*, int, int, int, int, int, int))installer_instance_click;
            inst->handle_key = (int (*)(void*, char, unsigned char))installer_instance_key;
            installer_instance_init((installer_state_t*)inst->state);
            installer_open = 1;
            break;

        default:
            return 0;
    }

    inst->pid = task_create(inst->title, 0, 1, app_type);
    active_instance_id = inst->instance_id;
    active_app_id = app_type;
    drag = 1;
    taskbar_set_app_minimized(0);
    draw_window();
    play_sound(750); sleep(30); play_sound(1100); sleep(40); no_sound();
    return inst->instance_id;
}

int maxp_launch_app(int app_type) {
    return maxp_spawn_instance(app_type, 0, 0);
}

int maxp_close_instance(int instance_id) {
    app_instance_t* inst = maxp_get_instance(instance_id);
    if (!inst) return 0;

    int closed_type = inst->app_type;
    if (inst->close) {
        inst->close(inst->state);
    }
    if (inst->pid > 0) {
        task_kill(inst->pid);
    }

    inst->instance_id = 0;

    // Check if any instances of closed_type remain
    int remaining = 0;
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].app_type == closed_type) {
            remaining = 1;
            break;
        }
    }
    if (!remaining) {
        if (closed_type == MAXP_APP_NOTEPAD) notepad_open = 0;
        else if (closed_type == MAXP_APP_CALC) calc_open = 0;
        else if (closed_type == MAXP_APP_PONG) pong_open = 0;
        else if (closed_type == MAXP_APP_EXPLORER) explorer_open = 0;
        else if (closed_type == MAXP_APP_SYSINFO) sysinfo_open = 0;
        else if (closed_type == MAXP_APP_MEM || closed_type == MAXP_APP_STRESS) mem_open = 0;
        else if (closed_type == MAXP_APP_INSTALLER) installer_open = 0;
    }

    if (active_instance_id == instance_id) {
        int next_id = 0;
        for (int i = 0; i < MAX_APP_INSTANCES; i++) {
            if (instances[i].instance_id != 0) {
                next_id = instances[i].instance_id;
                break;
            }
        }
        maxp_set_active_instance(next_id);
    }

    drag = 0;
    draw_window();
    play_sound(350); sleep(40); no_sound();
    return 1;
}

void maxp_close_app(int app_id) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].app_type == app_id) {
            maxp_close_instance(instances[i].instance_id);
        }
    }
}

void maxp_close_all_windows(void) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0) {
            maxp_close_instance(instances[i].instance_id);
        }
    }
    active_instance_id = 0;
    active_app_id = MAXP_APP_NONE;
}

void maxp_draw_active_instance(void) {
    app_instance_t* inst = maxp_get_active_instance();
    if (inst && !inst->is_minimized && inst->draw) {
        inst->draw(inst->state, inst->win_x, inst->win_y, inst->win_w, inst->win_h);
    }
}

void maxp_draw_all_instances(void) {
    // 1. Draw background windows (inactive)
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].instance_id != active_instance_id && !instances[i].is_minimized && instances[i].draw) {
            instances[i].draw(instances[i].state, instances[i].win_x, instances[i].win_y, instances[i].win_w, instances[i].win_h);
        }
    }
    // 2. Draw active window on top
    app_instance_t* act = maxp_get_active_instance();
    if (act && !act->is_minimized && act->draw) {
        act->draw(act->state, act->win_x, act->win_y, act->win_w, act->win_h);
    }
}

int maxp_handle_click_active(int mx, int my) {
    app_instance_t* inst = maxp_get_active_instance();
    if (inst && !inst->is_minimized && inst->handle_click) {
        int res = inst->handle_click(inst->state, inst->win_x, inst->win_y, inst->win_w, inst->win_h, mx, my);
        if (res == -1) {
            maxp_close_instance(inst->instance_id);
            return 1;
        }
        if (res != 0) return res;
    }

    // Check if clicked another running window in background to bring to focus!
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].instance_id != active_instance_id && !instances[i].is_minimized) {
            app_instance_t* bg_inst = &instances[i];
            if (mx >= bg_inst->win_x && mx <= bg_inst->win_x + bg_inst->win_w &&
                my >= bg_inst->win_y && my <= bg_inst->win_y + bg_inst->win_h) {
                maxp_set_active_instance(bg_inst->instance_id);
                draw_window();
                draw_cursor(pos_x, pos_y);
                play_sound(750); sleep(20); no_sound();
                return 1;
            }
        }
    }

    // Swallow any click falling inside the active window rectangle so it doesn't click-through to desktop icons underneath
    if (inst && !inst->is_minimized) {
        if (mx >= inst->win_x && mx <= inst->win_x + inst->win_w &&
            my >= inst->win_y && my <= inst->win_y + inst->win_h) {
            return 1;
        }
    }

    return 0;
}

int maxp_handle_key_active(char ch, unsigned char scan) {
    app_instance_t* inst = maxp_get_active_instance();
    if (inst && !inst->is_minimized && inst->handle_key) {
        int res = inst->handle_key(inst->state, ch, scan);
        if (res == -1) {
            maxp_close_instance(inst->instance_id);
            return 1;
        }
        return res;
    }
    return 0;
}

void maxp_tick_all_instances(void) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && instances[i].tick) {
            instances[i].tick(instances[i].state, instances[i].win_x, instances[i].win_y, instances[i].win_w, instances[i].win_h);
        }
    }
}

int maxp_has_ticking_instances(void) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        if (instances[i].instance_id != 0 && !instances[i].is_minimized && instances[i].tick) {
            return 1;
        }
    }
    return 0;
}

int maxp_launch_binary(const char* filename, const void* data, unsigned int size) {
    (void)filename;
    if (size < sizeof(maxb_header_t)) return 0;
    const maxb_header_t* hdr = (const maxb_header_t*)data;
    if (hdr->magic != MAXB_MAGIC) return 0;
    return maxp_spawn_instance(hdr->app_type, hdr->name, 0);
}

int maxp_launch_file(const char* filename) {
    int len = 0;
    while (filename[len] != '\0') len++;

    // Check for standalone binary executable (.bin)
    if (len >= 4 && (filename[len-4] == '.') &&
        (filename[len-3] == 'b' || filename[len-3] == 'B') &&
        (filename[len-2] == 'i' || filename[len-2] == 'I') &&
        (filename[len-1] == 'n' || filename[len-1] == 'N')) {
        int slot = maxfs_find_file(filename);
        if (slot >= 0) {
            static unsigned char bin_buf[2048];
            int read_bytes = maxfs_read_binary(filename, bin_buf, sizeof(bin_buf));
            if (read_bytes >= (int)sizeof(maxb_header_t)) {
                return maxp_launch_binary(filename, bin_buf, read_bytes);
            }
        }
    }

    // Check for .maxP files
    if (str_equal(filename, "notepad.maxP") || str_equal(filename, "Notepad.maxP")) {
        return maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", 0);
    }
    if (str_equal(filename, "explorer.maxP") || str_equal(filename, "Explorer.maxP")) {
        return maxp_spawn_instance(MAXP_APP_EXPLORER, "Explorer", 0);
    }
    if (str_equal(filename, "calc.maxP") || str_equal(filename, "Calc.maxP")) {
        return maxp_spawn_instance(MAXP_APP_CALC, "Calculator", 0);
    }
    if (str_equal(filename, "sysinfo.maxP") || str_equal(filename, "Sysinfo.maxP")) {
        return maxp_spawn_instance(MAXP_APP_SYSINFO, "SysInfo", 0);
    }
    if (str_equal(filename, "pong.maxP") || str_equal(filename, "Pong.maxP")) {
        return maxp_spawn_instance(MAXP_APP_PONG, "Pong Arcade", 0);
    }
    if (str_equal(filename, "install.maxP") || str_equal(filename, "Install.maxP")) {
        return maxp_spawn_instance(MAXP_APP_INSTALLER, "Installer", 0);
    }
    if (str_equal(filename, "mem.maxP") || str_equal(filename, "Mem.maxP")) {
        return maxp_spawn_instance(MAXP_APP_MEM, "Mem Monitor", 0);
    }
    if (str_equal(filename, "stress.maxP") || str_equal(filename, "Stress.maxP")) {
        return maxp_spawn_instance(MAXP_APP_STRESS, "RAM Stress Test", 0);
    }

    // Check custom .maxP content
    int slot = maxfs_find_file(filename);
    if (slot != -1) {
        struct VirtualFile* vf = maxfs_get_file(slot);
        if (vf && vf->exists) {
            const char* c = vf->content;
            int idx = 0;
            while (c[idx] != '\0' && idx < 500) {
                if (c[idx] == 'E' && c[idx+1] == 'X' && c[idx+2] == 'E' && c[idx+3] == 'C' && c[idx+4] == '=') {
                    const char* target = &c[idx + 5];
                    if (target[0] == 'n' && target[1] == 'o') return maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", 0);
                    if (target[0] == 'e' && target[1] == 'x') return maxp_spawn_instance(MAXP_APP_EXPLORER, "Explorer", 0);
                    if (target[0] == 'c' && target[1] == 'a') return maxp_spawn_instance(MAXP_APP_CALC, "Calculator", 0);
                    if (target[0] == 's' && target[1] == 'y') return maxp_spawn_instance(MAXP_APP_SYSINFO, "SysInfo", 0);
                    if (target[0] == 'p' && target[1] == 'o') return maxp_spawn_instance(MAXP_APP_PONG, "Pong", 0);
                    if (target[0] == 'i' && target[1] == 'n') return maxp_spawn_instance(MAXP_APP_INSTALLER, "Installer", 0);
                    if (target[0] == 'm' && target[1] == 'e') return maxp_spawn_instance(MAXP_APP_MEM, "Mem", 0);
                    if (target[0] == 's' && target[1] == 't') return maxp_spawn_instance(MAXP_APP_STRESS, "Stress", 0);
                }
                idx++;
            }
        }
    }

    // Default to Notepad
    return maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", filename);
}

void maxp_init(void) {
    for (int i = 0; i < MAX_APP_INSTANCES; i++) {
        instances[i].instance_id = 0;
    }
    active_instance_id = 0;
    active_app_id = MAXP_APP_NONE;

    // Seed Standalone Binary Executables into maxFS
    if (maxfs_find_file("notepad.bin") == -1) {
        maxfs_write_binary("notepad.bin", notepad_bin_data, notepad_bin_len);
    }
    if (maxfs_find_file("calc.bin") == -1) {
        maxfs_write_binary("calc.bin", calc_bin_data, calc_bin_len);
    }
    if (maxfs_find_file("pong.bin") == -1) {
        maxfs_write_binary("pong.bin", pong_bin_data, pong_bin_len);
    }
    if (maxfs_find_file("sysinfo.bin") == -1) {
        maxfs_write_binary("sysinfo.bin", sysinfo_bin_data, sysinfo_bin_len);
    }
    if (maxfs_find_file("mem.bin") == -1) {
        maxfs_write_binary("mem.bin", mem_bin_data, mem_bin_len);
    }
    if (maxfs_find_file("stress.bin") == -1) {
        maxfs_write_binary("stress.bin", stress_bin_data, stress_bin_len);
    }
    if (maxfs_find_file("explorer.bin") == -1) {
        maxfs_write_binary("explorer.bin", explorer_bin_data, explorer_bin_len);
    }
    if (maxfs_find_file("installer.bin") == -1) {
        maxfs_write_binary("installer.bin", installer_bin_data, installer_bin_len);
    }

    // Seed default .maxP program files if not present
    if (maxfs_find_file("notepad.maxP") == -1) {
        const char* np_content = "MAXP\nNAME=Notepad\nEXEC=notepad\nICON=NP\nDESC=maxOS Notepad 3.2 Text Editor\n";
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
    if (maxfs_find_file("stress.maxP") == -1) {
        const char* stress_content = "MAXP\nNAME=StressTest\nEXEC=stress\nICON=RAM\nDESC=RAM Hardware Stress Test & Benchmark\n";
        maxfs_write_file("stress.maxP", stress_content, 79);
    }
    if (maxfs_find_file("readme.txt") == -1) {
        const char* rm = "Welcome to maxOS RedCycle v3.2 x86_64!\nStandalone binaries: .bin (MAXB format)\nMulti-instance enabled: run multiple Notepads, Calcs, etc!\n";
        maxfs_write_file("readme.txt", rm, 137);
    }
}

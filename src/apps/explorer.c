#include "explorer.h"
#include "notepad.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"

#include "kernel.h"

extern int km_mode;

int explorer_open = 0;
static int selected_file = -1;
static char exp_status[64] = "Click file | [Open in Note] | [Delete] | F2 exit";

void explorer_init(void) {
    explorer_open = 0;
    selected_file = -1;
}

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 6, by + 4, text_col);
}

static int str_ends_with(const char* str, const char* suffix) {
    int slen = 0, suflen = 0;
    while (str[slen] != '\0') slen++;
    while (suffix[suflen] != '\0') suflen++;
    if (suflen > slen) return 0;
    for (int i = 0; i < suflen; i++) {
        if (str[slen - suflen + i] != suffix[i]) return 0;
    }
    return 1;
}

void explorer_draw(void) {
    prev_cursor();

    int exp_x = win_x + 15;
    int exp_y = win_y + 35;
    int exp_w = win_w - 30;
    int exp_h = win_h - 45;

    // Window frame
    draw_rect(exp_x, exp_y, exp_w, exp_h, 0x0000);
    draw_rect(exp_x + 1, exp_y + 1, exp_w - 2, exp_h - 2, 0xEF59);

    // Titlebar
    draw_rect(exp_x + 2, exp_y + 2, exp_w - 4, 20, 0x0320); // Dark Teal
    print_string("maxOS Explorer 3.1 - Filesystem & Disk Manager", exp_x + 8, exp_y + 6, 0xFFFF);

    // Toolbar (Row 1)
    draw_rect(exp_x + 2, exp_y + 22, exp_w - 4, 25, 0xDF17);
    draw_rect(exp_x + 2, exp_y + 46, exp_w - 4, 1, 0x9CD3);

    // Buttons
    draw_ui_btn(exp_x + 8,          exp_y + 25, 96,  19, "Open Note",   0x3DF2, 0x0000);
    draw_ui_btn(exp_x + 110,        exp_y + 25, 88,  19, "New Note",    0x24EE, 0xFFFF);
    draw_ui_btn(exp_x + 204,        exp_y + 25, 70,  19, "Delete",      0xF800, 0xFFFF);
    draw_ui_btn(exp_x + 280,        exp_y + 25, 80,  19, "Refresh",     0xC618, 0x0000);
    draw_ui_btn(exp_x + 366,        exp_y + 25, 115, 19, "Format Disk", 0x8000, 0xFFFF);
    draw_ui_btn(exp_x + exp_w - 68, exp_y + 25, 60,  19, "Close",       0xF9A6, 0x0000);

    // Drive Info Banner (Row 2)
    draw_rect(exp_x + 2, exp_y + 47, exp_w - 4, 20, 0xE71C);

    int total_files = 0;
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (ram_disk[i].exists) total_files++;
    }

    char banner[80];
    banner[0] = 'D'; banner[1] = 'i'; banner[2] = 's'; banner[3] = 'k'; banner[4] = ':'; banner[5] = ' ';
    int bp = 6;
    if (maxfs_is_mounted()) {
        const char* vol = maxfs_get_volume_label();
        for (int i = 0; vol[i] != '\0' && bp < 35; i++) banner[bp++] = vol[i];
    } else {
        banner[bp++] = 'R'; banner[bp++] = 'A'; banner[bp++] = 'M';
    }
    banner[bp++] = ' '; banner[bp++] = '|'; banner[bp++] = ' ';
    banner[bp++] = 'F'; banner[bp++] = 'i'; banner[bp++] = 'l'; banner[bp++] = 'e'; banner[bp++] = 's'; banner[bp++] = ':'; banner[bp++] = ' ';
    char cnt_str[8]; int_str(total_files, cnt_str);
    for (int i = 0; cnt_str[i] != '\0'; i++) banner[bp++] = cnt_str[i];
    banner[bp++] = '/'; banner[bp++] = '3'; banner[bp++] = '2';
    banner[bp++] = ' '; banner[bp++] = '|'; banner[bp++] = ' ';
    banner[bp++] = 'S'; banner[bp++] = 't'; banner[bp++] = 'a'; banner[bp++] = 't'; banner[bp++] = 'u'; banner[bp++] = 's'; banner[bp++] = ':'; banner[bp++] = ' ';
    if (maxfs_is_mounted()) {
        banner[bp++] = 'A'; banner[bp++] = 'T'; banner[bp++] = 'A'; banner[bp++] = ' ';
        banner[bp++] = 'O'; banner[bp++] = 'K';
    } else {
        banner[bp++] = 'U'; banner[bp++] = 'n'; banner[bp++] = 'm'; banner[bp++] = 'o'; banner[bp++] = 'u'; banner[bp++] = 'n'; banner[bp++] = 't'; banner[bp++] = 'e'; banner[bp++] = 'd';
    }
    banner[bp] = '\0';
    print_string(banner, exp_x + 8, exp_y + 52, 0x001F);

    // File Table Area
    int tbl_x = exp_x + 10;
    int tbl_y = exp_y + 72;
    int tbl_w = exp_w - 20;
    int tbl_h = exp_h - 100;

    draw_rect(tbl_x, tbl_y, tbl_w, tbl_h, 0xFFFF);
    draw_rect(tbl_x, tbl_y, tbl_w, 1, 0x7BEF);
    draw_rect(tbl_x, tbl_y, 1, tbl_h, 0x7BEF);
    draw_rect(tbl_x, tbl_y + tbl_h - 1, tbl_w, 1, 0x7BEF);
    draw_rect(tbl_x + tbl_w - 1, tbl_y, 1, tbl_h, 0x7BEF);

    // Table Header Row
    draw_rect(tbl_x + 1, tbl_y + 1, tbl_w - 2, 20, 0xDF17);
    draw_rect(tbl_x + 1, tbl_y + 21, tbl_w - 2, 1, 0x7BEF);
    print_string("Type",       tbl_x + 10,  tbl_y + 6, 0x0000);
    print_string("File Name",  tbl_x + 65,  tbl_y + 6, 0x0000);
    print_string("Size",       tbl_x + 280, tbl_y + 6, 0x0000);
    print_string("Location",   tbl_x + 390, tbl_y + 6, 0x0000);
    print_string("Status",     tbl_x + 510, tbl_y + 6, 0x0000);

    // List Files
    int row_y = tbl_y + 24;
    int rows_drawn = 0;
    for (int i = 0; i < MAXFS_MAX_FILES && rows_drawn < 14; i++) {
        struct VirtualFile* vf = maxfs_get_file(i);
        if (vf && vf->exists) {
            // Row background
            if (selected_file == i) {
                draw_rect(tbl_x + 2, row_y, tbl_w - 4, 20, 0x861F); // Soft Cyan highlight
            } else if (rows_drawn % 2 == 1) {
                draw_rect(tbl_x + 2, row_y, tbl_w - 4, 20, 0xF7BE); // Subtle zebra striping
            }

            // Selection indicator
            if (selected_file == i) {
                print_string(">", tbl_x + 3, row_y + 4, 0x0000);
            }

            // Type icon
            const char* type_str = "[TXT]";
            unsigned short type_col = 0x24EE; // Blue
            if (str_ends_with(vf->name, ".cfg") || str_ends_with(vf->name, ".ini")) {
                type_str = "[CFG]"; type_col = 0xFD20;
            } else if (str_ends_with(vf->name, ".bin") || str_ends_with(vf->name, ".iso")) {
                type_str = "[BIN]"; type_col = 0x981F;
            } else if (str_ends_with(vf->name, ".maxP") || str_ends_with(vf->name, ".maxp")) {
                type_str = "[maxP]"; type_col = 0x05E5;
            } else if (str_ends_with(vf->name, ".mapp") || str_ends_with(vf->name, ".app")) {
                type_str = "[APP]"; type_col = 0x05E5;
            }
            print_string((char*)type_str, tbl_x + 14, row_y + 4, type_col);

            // Name
            print_string(vf->name, tbl_x + 65, row_y + 4, 0x0000);

            // Size
            char sz_str[16];
            int_str(vf->size, sz_str);
            int sp = 0; while (sz_str[sp] != '\0') sp++;
            sz_str[sp++] = ' '; sz_str[sp++] = 'B'; sz_str[sp] = '\0';
            print_string(sz_str, tbl_x + 280, row_y + 4, 0x0320);

            // LBA Location
            struct DiskInode* in = maxfs_get_inode(i);
            if (in && in->start_lba > 0) {
                char lba_str[24] = "LBA ";
                int lp = 4;
                char num_str[16]; int_str(in->start_lba, num_str);
                for (int b = 0; num_str[b] != '\0'; b++) lba_str[lp++] = num_str[b];
                lba_str[lp] = '\0';
                print_string(lba_str, tbl_x + 390, row_y + 4, 0x001F);
            } else {
                print_string("RAM", tbl_x + 390, row_y + 4, 0x7BEF);
            }

            // Status
            print_string("Ready", tbl_x + 510, row_y + 4, 0x03EA);

            row_y += 22;
            rows_drawn++;
        }
    }

    if (total_files == 0) {
        print_string("No files found on maxFS.", tbl_x + 30, tbl_y + 50, 0xF800);
        print_string("Click [New Note] to create your first file!", tbl_x + 30, tbl_y + 70, 0x0000);
    }

    // Footer Help Bar
    draw_rect(exp_x + 2, exp_y + exp_h - 24, exp_w - 4, 22, 0xDF17);
    draw_rect(exp_x + 2, exp_y + exp_h - 24, exp_w - 4, 1, 0x9CD3);
    print_string(exp_status, exp_x + 10, exp_y + exp_h - 18, 0x0000);

    draw_cursor(pos_x, pos_y);
}

void explorer_open_window(void) {
    explorer_open = 1;
    drag = 1;
    maxp_set_active_app(MAXP_APP_EXPLORER);
    if (maxfs_is_mounted()) {
        maxfs_mount(); // Resync directory from disk
    }
    selected_file = -1;
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (ram_disk[i].exists) {
            selected_file = i;
            break;
        }
    }
    draw_window();
    play_sound(550); sleep(60); no_sound();
}

void explorer_close_window(void) {
    play_sound(350); sleep(60); no_sound();
    maxp_close_app(MAXP_APP_EXPLORER);
}

int explorer_handle_click(int mouse_x, int mouse_y) {
    if (!explorer_open) return 0;

    int exp_x = win_x + 15;
    int exp_y = win_y + 35;
    int exp_w = win_w - 30;
    int exp_h = win_h - 45;

    // Check [Open Note] button: exp_x + 8 .. exp_x + 104, exp_y + 25 .. exp_y + 44
    if (mouse_x >= exp_x + 8 && mouse_x <= exp_x + 104 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        if (selected_file >= 0 && selected_file < MAXFS_MAX_FILES && ram_disk[selected_file].exists) {
            char chosen_name[32];
            int p = 0;
            while (ram_disk[selected_file].name[p] != '\0' && p < 31) {
                chosen_name[p] = ram_disk[selected_file].name[p];
                p++;
            }
            chosen_name[p] = '\0';

            explorer_open = 0;
            notepad_open_file_by_name(chosen_name);
        } else {
            play_sound(250); sleep(80); no_sound();
        }
        return 1;
    }

    // Check [New Note] button: exp_x + 110 .. exp_x + 198
    if (mouse_x >= exp_x + 110 && mouse_x <= exp_x + 198 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        explorer_open = 0;
        notepad_open_window();
        return 1;
    }

    // Check [Delete] button: exp_x + 204 .. exp_x + 274
    if (mouse_x >= exp_x + 204 && mouse_x <= exp_x + 274 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        if (selected_file >= 0 && selected_file < MAXFS_MAX_FILES && ram_disk[selected_file].exists) {
            maxfs_delete_file(ram_disk[selected_file].name);
            selected_file = -1;
            play_sound(300); sleep(80); play_sound(200); sleep(80); no_sound();
            explorer_draw();
        }
        return 1;
    }

    // Check [Refresh] button: exp_x + 280 .. exp_x + 360
    if (mouse_x >= exp_x + 280 && mouse_x <= exp_x + 360 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        if (maxfs_is_mounted()) maxfs_mount();
        explorer_draw();
        play_sound(700); sleep(40); no_sound();
        return 1;
    }

    // Check [Format Disk] button: exp_x + 366 .. exp_x + 481
    if (mouse_x >= exp_x + 366 && mouse_x <= exp_x + 481 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        maxfs_format("maxOS System Disk");
        selected_file = -1;
        explorer_draw();
        play_sound(400); sleep(60); play_sound(800); sleep(60); no_sound();
        return 1;
    }

    // Check [Close] button: exp_x + exp_w - 68 .. exp_x + exp_w - 8
    if (mouse_x >= exp_x + exp_w - 68 && mouse_x <= exp_x + exp_w - 8 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        explorer_close_window();
        return 1;
    }

    // Check click on file table row
    int tbl_x = exp_x + 10;
    int tbl_y = exp_y + 72;
    int tbl_w = exp_w - 20;

    int row_y = tbl_y + 24;
    int rows_drawn = 0;
    for (int i = 0; i < MAXFS_MAX_FILES && rows_drawn < 14; i++) {
        if (ram_disk[i].exists) {
            if (mouse_x >= tbl_x && mouse_x <= tbl_x + tbl_w &&
                mouse_y >= row_y && mouse_y <= row_y + 22) {
                if (selected_file == i) {
                    // Second click opens file directly in Notepad!
                    char chosen_name[32];
                    int p = 0;
                    while (ram_disk[i].name[p] != '\0' && p < 31) {
                        chosen_name[p] = ram_disk[i].name[p];
                        p++;
                    }
                    chosen_name[p] = '\0';
                    explorer_open = 0;
                    if (maxp_is_maxp_file(chosen_name)) {
                        maxp_launch_file(chosen_name);
                    } else {
                        notepad_open_file_by_name(chosen_name);
                    }
                    return 1;
                }
                selected_file = i;
                explorer_draw();
                play_sound(600); sleep(40); no_sound();
                return 1;
            }
            row_y += 22;
            rows_drawn++;
        }
    }

    // Swallow any click inside explorer window
    if (mouse_x >= exp_x && mouse_x <= exp_x + exp_w && mouse_y >= exp_y && mouse_y <= exp_y + exp_h) {
        return 1;
    }

    return 0;
}

int explorer_handle_key(char ascii_char, unsigned char scan_code) {
    if (!explorer_open) return 0;

    // F2 (0x3C), Escape (0x01), 'c', 'C', 'q', 'Q', 'S'
    if (scan_code == 0x01 || scan_code == 0x3C || ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || ascii_char == 'Q' || ascii_char == 'S') {
        explorer_close_window();
        return 1;
    }

    // F1: open new Notepad
    if (scan_code == 0x3B) {
        explorer_open = 0;
        notepad_open_window();
        return 1;
    }

    // Enter: open selected file in Notepad
    if (scan_code == 0x1C || ascii_char == '\n') {
        if (selected_file >= 0 && selected_file < MAXFS_MAX_FILES && ram_disk[selected_file].exists) {
            char chosen_name[32];
            int p = 0;
            while (ram_disk[selected_file].name[p] != '\0' && p < 31) {
                chosen_name[p] = ram_disk[selected_file].name[p];
                p++;
            }
            chosen_name[p] = '\0';
            explorer_open = 0;
            if (maxp_is_maxp_file(chosen_name)) {
                maxp_launch_file(chosen_name);
            } else {
                notepad_open_file_by_name(chosen_name);
            }
            return 1;
        }
        return 1;
    }

    // Delete key (0x53)
    if (scan_code == 0x53) {
        if (selected_file >= 0 && selected_file < MAXFS_MAX_FILES && ram_disk[selected_file].exists) {
            maxfs_delete_file(ram_disk[selected_file].name);
            selected_file = -1;
            explorer_draw();
            play_sound(300); sleep(80); no_sound();
            return 1;
        }
    }

    // Up / Down arrow selection
    if (scan_code == 0x48 || ascii_char == 'U') {
        // Prev file
        int prev = -1;
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            if (ram_disk[i].exists) {
                if (selected_file == -1 || i < selected_file) prev = i;
            }
        }
        if (prev != -1) {
            selected_file = prev;
            explorer_draw();
            play_sound(550); sleep(30); no_sound();
        }
        return 1;
    }

    if (scan_code == 0x50 || ascii_char == 'D') {
        // Next file
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            if (ram_disk[i].exists && (selected_file == -1 || i > selected_file)) {
                selected_file = i;
                explorer_draw();
                play_sound(550); sleep(30); no_sound();
                break;
            }
        }
        return 1;
    }

    return 0;
}

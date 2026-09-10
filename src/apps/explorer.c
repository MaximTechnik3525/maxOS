#include "explorer.h"
#include "notepad.h"
#include "maxfs.h"
#include "ata.h"
#include "maxp.h"
#include "kernel.h"

int explorer_open = 0;
static explorer_state_t primary_explorer_state;

static void explorer_set_status_s(explorer_state_t* s, const char* msg) {
    strncpy(s->exp_status, msg, sizeof(s->exp_status) - 1);
    s->exp_status[sizeof(s->exp_status) - 1] = '\0';
}

void explorer_instance_init(explorer_state_t* s) {
    s->current_dir_inode = 0;
    s->selected_index = -1;
    s->scroll_offset = 0;
    s->dlg.active = 0;
    explorer_set_status_s(s, "Click file | [Open] | [Delete] | Scroll [^][v] / PgUp/PgDn | Esc exit");
}

void explorer_instance_draw(explorer_state_t* s, int exp_x, int exp_y, int exp_w, int exp_h) {
    prev_cursor();

    // Window frame
    draw_rect(exp_x, exp_y, exp_w, exp_h, 0x0000);
    draw_rect(exp_x + 1, exp_y + 1, exp_w - 2, exp_h - 2, 0xEF59);

    // Titlebar
    draw_rect(exp_x + 2, exp_y + 2, exp_w - 4, 20, 0x0320);
    print_string("maxOS Explorer 3.5 - Filesystem & Disk Manager", exp_x + 8, exp_y + 6, 0xFFFF);

    // [_] Minimize & [X] Close button (Titlebar)
    draw_ui_btn(exp_x + exp_w - 44, exp_y + 4, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(exp_x + exp_w - 22, exp_y + 4, 18, 16, "X", 0xF800, 0xFFFF);

    // Toolbar (Row 1)
    draw_rect(exp_x + 2, exp_y + 22, exp_w - 4, 25, 0xDF17);
    draw_rect(exp_x + 2, exp_y + 46, exp_w - 4, 1, 0x9CD3);

    // Buttons
    draw_ui_btn(exp_x + 8,          exp_y + 25, 40,  19, "Up",         0xFE60, 0x0000);
    draw_ui_btn(exp_x + 54,         exp_y + 25, 60,  19, "Open",       0x3DF2, 0x0000);
    draw_ui_btn(exp_x + 120,        exp_y + 25, 70,  19, "New File",   0x24EE, 0xFFFF);
    draw_ui_btn(exp_x + 196,        exp_y + 25, 70,  19, "New Dir",    0x05E5, 0xFFFF);
    draw_ui_btn(exp_x + 272,        exp_y + 25, 70,  19, "Rename",     0x8400, 0xFFFF);
    draw_ui_btn(exp_x + 348,        exp_y + 25, 60,  19, "Delete",     0xF800, 0xFFFF);
    draw_ui_btn(exp_x + 414,        exp_y + 25, 60,  19, "Refresh",    0xC618, 0x0000);
    draw_ui_btn(exp_x + 480,        exp_y + 25, 70,  19, "Format",     0x8000, 0xFFFF);
    draw_ui_btn(exp_x + 556,        exp_y + 25, 70,  19, "Read CD",    0x03EA, 0x0000);

    // Drive Info Banner (Row 2)
    draw_rect(exp_x + 2, exp_y + 47, exp_w - 4, 20, 0xE71C);

    // Get items in current directory
    int dir_indices[MAXFS_MAX_FILES];
    int dir_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dir_indices, MAXFS_MAX_FILES);

    char path_buf[64];
    maxfs_get_dir_path((unsigned int)s->current_dir_inode, path_buf, 60);

    char banner[80];
    int bp = 0;
    for (int i = 0; path_buf[i] != '\0' && bp < 40; i++) banner[bp++] = path_buf[i];
    banner[bp++] = ' '; banner[bp++] = '|'; banner[bp++] = ' ';
    char cnt_str[8]; int_str(dir_count, cnt_str);
    for (int i = 0; cnt_str[i] != '\0'; i++) banner[bp++] = cnt_str[i];
    banner[bp++] = ' '; banner[bp++] = 'i'; banner[bp++] = 't'; banner[bp++] = 'e'; banner[bp++] = 'm'; banner[bp++] = 's';
    if (dir_count > EXPLORER_MAX_VISIBLE) {
        banner[bp++] = ' '; banner[bp++] = '(';
        char from_str[8]; int_str(s->scroll_offset + 1, from_str);
        for (int i = 0; from_str[i] != '\0'; i++) banner[bp++] = from_str[i];
        banner[bp++] = '-';
        int to_idx = s->scroll_offset + EXPLORER_MAX_VISIBLE;
        if (to_idx > dir_count) to_idx = dir_count;
        char to_str[8]; int_str(to_idx, to_str);
        for (int i = 0; to_str[i] != '\0'; i++) banner[bp++] = to_str[i];
        banner[bp++] = ')';
    }
    banner[bp++] = ' '; banner[bp++] = '|'; banner[bp++] = ' ';
    if (maxfs_is_mounted()) {
        banner[bp++] = 'A'; banner[bp++] = 'T'; banner[bp++] = 'A';
    } else {
        banner[bp++] = 'R'; banner[bp++] = 'A'; banner[bp++] = 'M';
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

    // List items in current directory
    int row_y = tbl_y + 24;
    int max_visible = EXPLORER_MAX_VISIBLE;
    int sb_w = 18;
    int row_w = tbl_w - sb_w - 5;
    for (int di = s->scroll_offset; di < dir_count && (di - s->scroll_offset) < max_visible; di++) {
        int inode_idx = dir_indices[di];
        struct VirtualFile* vf = maxfs_get_file(inode_idx);
        if (!vf || !vf->exists) continue;
        int row_num = di - s->scroll_offset;

        // Row background
        if (s->selected_index == inode_idx) {
            draw_rect(tbl_x + 2, row_y, row_w, 20, 0x861F);
        } else if (row_num % 2 == 1) {
            draw_rect(tbl_x + 2, row_y, row_w, 20, 0xF7BE);
        }

        // Selection indicator
        if (s->selected_index == inode_idx) {
            print_string(">", tbl_x + 3, row_y + 4, 0x0000);
        }

        // Type icon
        const char* type_str = "[TXT]";
        unsigned short type_col = 0x24EE;
        if (vf->is_dir) {
            type_str = "[DIR]"; type_col = 0xFE60;
        } else if (str_ends_with(vf->name, ".cfg") || str_ends_with(vf->name, ".ini")) {
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
        if (vf->is_dir) {
            print_string("<DIR>", tbl_x + 280, row_y + 4, 0xFE60);
        } else {
            char sz_str[16];
            int_str(vf->size, sz_str);
            int sp = 0; while (sz_str[sp] != '\0') sp++;
            sz_str[sp++] = ' '; sz_str[sp++] = 'B'; sz_str[sp] = '\0';
            print_string(sz_str, tbl_x + 280, row_y + 4, 0x0320);
        }

        // LBA Location
        struct DiskInode* in = maxfs_get_inode(inode_idx);
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

        print_string("Ready", tbl_x + 510, row_y + 4, 0x03EA);

        row_y += 22;
    }

    if (dir_count == 0) {
        print_string("Empty directory.", tbl_x + 30, tbl_y + 50, 0xF800);
        print_string("Click [New Note] or [New Dir] to add items!", tbl_x + 30, tbl_y + 70, 0x0000);
    }

    // Scrollbar Column (Right side of table)
    int sb_x = tbl_x + tbl_w - sb_w - 1;
    int sb_y = tbl_y + 22;
    int sb_h = tbl_h - 23;

    // Track
    draw_rect(sb_x, sb_y, sb_w, sb_h, 0xE71C);
    draw_rect(sb_x, sb_y, 1, sb_h, 0x7BEF);

    // [^] Up Button
    draw_ui_btn(sb_x, sb_y, sb_w, 18, "^", 0xCE79, 0x0000);

    // [v] Down Button
    draw_ui_btn(sb_x, sb_y + sb_h - 18, sb_w, 18, "v", 0xCE79, 0x0000);

    // Thumb
    int track_y = sb_y + 19;
    int track_h = sb_h - 38;
    if (track_h > 8) {
        if (dir_count <= max_visible) {
            draw_3d_box(sb_x + 1, track_y, sb_w - 2, track_h, 0, 0xBDD7);
        } else {
            int thumb_h = (max_visible * track_h) / dir_count;
            if (thumb_h < 12) thumb_h = 12;
            int max_offset = dir_count - max_visible;
            if (max_offset < 1) max_offset = 1;
            int thumb_y = track_y + (s->scroll_offset * (track_h - thumb_h)) / max_offset;
            if (thumb_y + thumb_h > track_y + track_h) thumb_y = track_y + track_h - thumb_h;
            draw_3d_box(sb_x + 1, thumb_y, sb_w - 2, thumb_h, 0, 0x861F);
        }
    }

    // Footer Help Bar
    draw_rect(exp_x + 2, exp_y + exp_h - 24, exp_w - 4, 22, 0xDF17);
    draw_rect(exp_x + 2, exp_y + exp_h - 24, exp_w - 4, 1, 0x9CD3);
    print_string(s->exp_status, exp_x + 10, exp_y + exp_h - 18, 0x0000);

    if (s->dlg.active) {
        dialog_draw(&s->dlg);
    }

    draw_cursor(pos_x, pos_y);
}

int explorer_instance_click(explorer_state_t* s, int exp_x, int exp_y, int exp_w, int exp_h, int mouse_x, int mouse_y) {
    if (s->dlg.active) {
        dialog_handle_click(&s->dlg, mouse_x, mouse_y);
        if (s->dlg.result != DIALOG_RES_NONE) {
            if (s->dlg.result == DIALOG_RES_OK) {
                char new_name[32] = {0};
                const char* val = dialog_get_field(&s->dlg, 0);
                if (val) {
                    int cp = 0;
                    while (val[cp] && cp < 31) { new_name[cp] = val[cp]; cp++; }
                    new_name[cp] = '\0';
                }
                if (s->dlg.dialog_type == EXPLORER_DLG_NEW_FILE) {
                    maxfs_write_file_in((unsigned int)s->current_dir_inode, new_name, "", 0);
                } else if (s->dlg.dialog_type == EXPLORER_DLG_NEW_DIR) {
                    maxfs_create_dir(new_name, (unsigned int)s->current_dir_inode);
                } else if (s->dlg.dialog_type == EXPLORER_DLG_RENAME) {
                    if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
                        maxfs_rename(s->selected_index, new_name);
                    }
                }
            }
            s->dlg.active = 0;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // Check [Up] button
    if (mouse_x >= exp_x + 8 && mouse_x <= exp_x + 48 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 8, exp_y + 25, 40, 19, "Up", 0xFE60, 0x0000);
        if (s->current_dir_inode != 0) {
            s->current_dir_inode = (int)maxfs_get_parent(s->current_dir_inode);
            s->selected_index = -1;
            s->scroll_offset = 0;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // Check [Open] button
    if (mouse_x >= exp_x + 54 && mouse_x <= exp_x + 134 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 54, exp_y + 25, 80, 19, "Open", 0x3DF2, 0x0000);
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                if (vf->is_dir) {
                    s->current_dir_inode = s->selected_index;
                    s->selected_index = -1;
                    s->scroll_offset = 0;
                    maxp_draw_active_instance();
                } else if (maxp_is_maxp_file(vf->name) || str_ends_with(vf->name, ".bin") || str_ends_with(vf->name, ".BIN")) {
                    maxp_launch_file(vf->name);
                } else {
                    maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", vf->name);
                }
            }
        }
        return 1;
    }

    // Check [New File] button
    if (mouse_x >= exp_x + 120 && mouse_x <= exp_x + 190 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 120, exp_y + 25, 70, 19, "New File", 0x24EE, 0xFFFF);
        dialog_init(&s->dlg, EXPLORER_DLG_NEW_FILE, "Create New File", "Enter the new file's name:");
        dialog_add_field(&s->dlg, "File Name:", "newfile.txt", 31);
        dialog_center(&s->dlg, exp_x, exp_y, exp_w, exp_h);
        s->dlg.active = 1;
        maxp_draw_active_instance();
        return 1;
    }

    // Check [New Dir] button
    if (mouse_x >= exp_x + 196 && mouse_x <= exp_x + 266 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 196, exp_y + 25, 70, 19, "New Dir", 0x05E5, 0xFFFF);
        dialog_init(&s->dlg, EXPLORER_DLG_NEW_DIR, "Create New Directory", "Enter the new directory's name:");
        dialog_add_field(&s->dlg, "Dir Name:", "NewFolder", 31);
        dialog_center(&s->dlg, exp_x, exp_y, exp_w, exp_h);
        s->dlg.active = 1;
        maxp_draw_active_instance();
        return 1;
    }

    // Check [Rename] button
    if (mouse_x >= exp_x + 272 && mouse_x <= exp_x + 342 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 272, exp_y + 25, 70, 19, "Rename", 0x8400, 0xFFFF);
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                dialog_init(&s->dlg, EXPLORER_DLG_RENAME, "Rename File/Directory", "Enter the new name for the item:");
                dialog_add_field(&s->dlg, "New Name:", vf->name, 31);
                dialog_center(&s->dlg, exp_x, exp_y, exp_w, exp_h);
                s->dlg.active = 1;
                maxp_draw_active_instance();
            }
        }
        return 1;
    }

    // Check [Delete] button
    if (mouse_x >= exp_x + 348 && mouse_x <= exp_x + 408 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 348, exp_y + 25, 60, 19, "Delete", 0xF800, 0xFFFF);
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                maxfs_delete_inode(s->selected_index);
                s->selected_index = -1;
                play_sound(300); sleep(80); play_sound(200); sleep(80); no_sound();
                maxp_draw_active_instance();
            }
        }
        return 1;
    }

    // Check [Refresh] button
    if (mouse_x >= exp_x + 414 && mouse_x <= exp_x + 474 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 414, exp_y + 25, 60, 19, "Refresh", 0xC618, 0x0000);
        if (maxfs_is_mounted()) maxfs_mount();
        maxp_draw_active_instance();
        return 1;
    }

    // Check [Format Disk] button
    if (mouse_x >= exp_x + 480 && mouse_x <= exp_x + 550 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 480, exp_y + 25, 70, 19, "Format", 0x8000, 0xFFFF);
        maxfs_format("maxOS System Disk");
        s->selected_index = -1;
        maxp_draw_active_instance();
        return 1;
    }

    // Check [Read CD] button
    if (mouse_x >= exp_x + 556 && mouse_x <= exp_x + 626 && mouse_y >= exp_y + 25 && mouse_y <= exp_y + 44) {
        ui_btn_click_effect(exp_x + 556, exp_y + 25, 70, 19, "Read CD", 0x03EA, 0x0000);
        char cd_label[64] = {0};
        atapi_get_volume_label(cd_label);
        char status_msg[80] = "CD-ROM Vol: ";
        int sp = 12;
        for (int i = 0; cd_label[i] && sp < 79; i++) {
            status_msg[sp++] = cd_label[i];
        }
        status_msg[sp] = '\0';
        explorer_set_status_s(s, status_msg);
        maxp_draw_active_instance();
        return 1;
    }

    // Check [_] Minimize button (Titlebar)
    if (mouse_x >= exp_x + exp_w - 48 && mouse_x <= exp_x + exp_w - 26 && mouse_y >= exp_y && mouse_y <= exp_y + 24) {
        ui_btn_click_effect(exp_x + exp_w - 44, exp_y + 4, 18, 16, "_", 0xCE79, 0x0000);
        return -2; // Request minimize
    }

    // Check [X] Close button (Titlebar)
    if (mouse_x >= exp_x + exp_w - 26 && mouse_x <= exp_x + exp_w && mouse_y >= exp_y && mouse_y <= exp_y + 24) {
        ui_btn_click_effect(exp_x + exp_w - 22, exp_y + 4, 18, 16, "X", 0xF800, 0xFFFF);
        return -1; // Request close
    }

    // Table & Scrollbar dimensions
    int tbl_x = exp_x + 10;
    int tbl_y = exp_y + 72;
    int tbl_w = exp_w - 20;
    int tbl_h = exp_h - 100;
    int sb_w = 18;
    int sb_x = tbl_x + tbl_w - sb_w - 1;
    int sb_y = tbl_y + 22;
    int sb_h = tbl_h - 23;

    int dir_indices_c[MAXFS_MAX_FILES];
    int dir_count_c = maxfs_list_dir((unsigned int)s->current_dir_inode, dir_indices_c, MAXFS_MAX_FILES);

    // Scrollbar clicks
    // 1. Scroll Up button [^]
    if (mouse_x >= sb_x && mouse_x <= sb_x + sb_w && mouse_y >= sb_y && mouse_y <= sb_y + 18) {
        ui_btn_click_effect(sb_x, sb_y, sb_w, 18, "^", 0xCE79, 0x0000);
        if (s->scroll_offset > 0) {
            s->scroll_offset--;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // 2. Scroll Down button [v]
    if (mouse_x >= sb_x && mouse_x <= sb_x + sb_w && mouse_y >= sb_y + sb_h - 18 && mouse_y <= sb_y + sb_h) {
        ui_btn_click_effect(sb_x, sb_y + sb_h - 18, sb_w, 18, "v", 0xCE79, 0x0000);
        if (s->scroll_offset + EXPLORER_MAX_VISIBLE < dir_count_c) {
            s->scroll_offset++;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // 3. Track click (Page Up / Page Down)
    if (mouse_x >= sb_x && mouse_x <= sb_x + sb_w && mouse_y > sb_y + 18 && mouse_y < sb_y + sb_h - 18) {
        int track_y = sb_y + 19;
        int track_h = sb_h - 38;
        if (dir_count_c > EXPLORER_MAX_VISIBLE && track_h > 8) {
            int thumb_h = (EXPLORER_MAX_VISIBLE * track_h) / dir_count_c;
            if (thumb_h < 12) thumb_h = 12;
            int max_offset = dir_count_c - EXPLORER_MAX_VISIBLE;
            if (max_offset < 1) max_offset = 1;
            int thumb_y = track_y + (s->scroll_offset * (track_h - thumb_h)) / max_offset;
            if (mouse_y < thumb_y) {
                s->scroll_offset -= EXPLORER_MAX_VISIBLE;
                if (s->scroll_offset < 0) s->scroll_offset = 0;
            } else if (mouse_y > thumb_y + thumb_h) {
                s->scroll_offset += EXPLORER_MAX_VISIBLE;
                if (s->scroll_offset > dir_count_c - EXPLORER_MAX_VISIBLE) s->scroll_offset = dir_count_c - EXPLORER_MAX_VISIBLE;
            }
            maxp_draw_active_instance();
        }
        return 1;
    }

    // Check click on file table row
    int row_y = tbl_y + 24;
    for (int di = s->scroll_offset; di < dir_count_c && (di - s->scroll_offset) < EXPLORER_MAX_VISIBLE; di++) {
        int inode_idx = dir_indices_c[di];
        struct VirtualFile* vf = maxfs_get_file(inode_idx);
        if (vf && vf->exists) {
            if (mouse_x >= tbl_x && mouse_x < sb_x &&
                mouse_y >= row_y && mouse_y <= row_y + 22) {
                if (s->selected_index == inode_idx) {
                    if (vf->is_dir) {
                        s->current_dir_inode = inode_idx;
                        s->selected_index = -1;
                        s->scroll_offset = 0;
                    } else if (maxp_is_maxp_file(vf->name) || str_ends_with(vf->name, ".bin") || str_ends_with(vf->name, ".BIN")) {
                        maxp_launch_file(vf->name);
                    } else {
                        maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", vf->name);
                    }
                    maxp_draw_active_instance();
                    return 1;
                }
                s->selected_index = inode_idx;
                maxp_draw_active_instance();
                play_sound(600); sleep(40); no_sound();
                return 1;
            }
            row_y += 22;
        }
    }

    if (mouse_x >= exp_x && mouse_x <= exp_x + exp_w && mouse_y >= exp_y && mouse_y <= exp_y + exp_h) {
        return 1;
    }

    return 0;
}

int explorer_instance_key(explorer_state_t* s, char ascii_char, unsigned char scan_code) {
    if (s->dlg.active) {
        dialog_handle_key(&s->dlg, ascii_char, scan_code);
        if (s->dlg.result != DIALOG_RES_NONE) {
            if (s->dlg.result == DIALOG_RES_OK) {
                char new_name[32] = {0};
                const char* val = dialog_get_field(&s->dlg, 0);
                if (val) {
                    int cp = 0;
                    while (val[cp] && cp < 31) { new_name[cp] = val[cp]; cp++; }
                    new_name[cp] = '\0';
                }
                if (s->dlg.dialog_type == EXPLORER_DLG_NEW_FILE) {
                    maxfs_write_file_in((unsigned int)s->current_dir_inode, new_name, "", 0);
                } else if (s->dlg.dialog_type == EXPLORER_DLG_NEW_DIR) {
                    maxfs_create_dir(new_name, (unsigned int)s->current_dir_inode);
                } else if (s->dlg.dialog_type == EXPLORER_DLG_RENAME) {
                    if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
                        maxfs_rename(s->selected_index, new_name);
                    }
                }
            }
            s->dlg.active = 0;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // Escape (0x01), 'c', 'C', 'q', 'Q'
    if (scan_code == 0x01 || ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || ascii_char == 'Q') {
        return -1; // Request close
    }

    // F2 (0x3C): Rename
    if (scan_code == 0x3C) {
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                dialog_init(&s->dlg, EXPLORER_DLG_RENAME, "Rename File/Directory", "Enter the new name for the item:");
                dialog_add_field(&s->dlg, "New Name:", vf->name, 31);
                // Center dialog. We don't have exp_x, exp_y in key handler, so use fixed screen size or primary explorer size
                dialog_center(&s->dlg, 15, 35, 750, 555); // Approximated from typical win_x + 15, etc.
                s->dlg.active = 1;
                maxp_draw_active_instance();
            }
        }
        return 1;
    }

    // F1: open new Notepad
    if (scan_code == 0x3B) {
        maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", 0);
        return 1;
    }

    // Backspace (0x0E): go to parent directory
    if (scan_code == 0x0E) {
        if (s->current_dir_inode != 0) {
            s->current_dir_inode = (int)maxfs_get_parent(s->current_dir_inode);
            s->selected_index = -1;
            s->scroll_offset = 0;
            maxp_draw_active_instance();
        }
        return 1;
    }

    // Enter: open selected file or enter directory
    if (scan_code == 0x1C || ascii_char == '\n') {
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                if (vf->is_dir) {
                    s->current_dir_inode = s->selected_index;
                    s->selected_index = -1;
                    s->scroll_offset = 0;
                    maxp_draw_active_instance();
                } else if (maxp_is_maxp_file(vf->name) || str_ends_with(vf->name, ".bin") || str_ends_with(vf->name, ".BIN")) {
                    maxp_launch_file(vf->name);
                } else {
                    maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", vf->name);
                }
                return 1;
            }
        }
        return 1;
    }

    // Delete key (0x53)
    if (scan_code == 0x53) {
        if (s->selected_index >= 0 && s->selected_index < MAXFS_MAX_FILES) {
            struct VirtualFile* vf = maxfs_get_file(s->selected_index);
            if (vf && vf->exists) {
                maxfs_delete_inode(s->selected_index);
                s->selected_index = -1;
                maxp_draw_active_instance();
                play_sound(300); sleep(80); no_sound();
                return 1;
            }
        }
    }

    // PageUp (0x49): scroll up
    if (scan_code == 0x49) {
        int dk_indices[MAXFS_MAX_FILES];
        int dk_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dk_indices, MAXFS_MAX_FILES);
        s->scroll_offset -= EXPLORER_MAX_VISIBLE;
        if (s->scroll_offset < 0) s->scroll_offset = 0;
        if (dk_count > 0 && s->scroll_offset < dk_count) {
            s->selected_index = dk_indices[s->scroll_offset];
        }
        maxp_draw_active_instance();
        play_sound(500); sleep(20); no_sound();
        return 1;
    }

    // PageDown (0x51): scroll down
    if (scan_code == 0x51) {
        int dk_indices[MAXFS_MAX_FILES];
        int dk_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dk_indices, MAXFS_MAX_FILES);
        if (dk_count > EXPLORER_MAX_VISIBLE) {
            s->scroll_offset += EXPLORER_MAX_VISIBLE;
            if (s->scroll_offset > dk_count - EXPLORER_MAX_VISIBLE) s->scroll_offset = dk_count - EXPLORER_MAX_VISIBLE;
        }
        if (dk_count > 0 && s->scroll_offset < dk_count) {
            s->selected_index = dk_indices[s->scroll_offset];
        }
        maxp_draw_active_instance();
        play_sound(500); sleep(20); no_sound();
        return 1;
    }

    // Home (0x47): jump to first item
    if (scan_code == 0x47) {
        int dk_indices[MAXFS_MAX_FILES];
        int dk_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dk_indices, MAXFS_MAX_FILES);
        s->scroll_offset = 0;
        if (dk_count > 0) s->selected_index = dk_indices[0];
        maxp_draw_active_instance();
        return 1;
    }

    // End (0x4F): jump to last item
    if (scan_code == 0x4F) {
        int dk_indices[MAXFS_MAX_FILES];
        int dk_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dk_indices, MAXFS_MAX_FILES);
        if (dk_count > EXPLORER_MAX_VISIBLE) s->scroll_offset = dk_count - EXPLORER_MAX_VISIBLE;
        else s->scroll_offset = 0;
        if (dk_count > 0) s->selected_index = dk_indices[dk_count - 1];
        maxp_draw_active_instance();
        return 1;
    }

    // Up / Down arrow selection with auto-scrolling
    {
        int dk_indices[MAXFS_MAX_FILES];
        int dk_count = maxfs_list_dir((unsigned int)s->current_dir_inode, dk_indices, MAXFS_MAX_FILES);
        int cur_di = -1;
        for (int d = 0; d < dk_count; d++) {
            if (dk_indices[d] == s->selected_index) { cur_di = d; break; }
        }

        if (scan_code == 0x48 || ascii_char == 'U') {
            if (dk_count > 0) {
                int new_di = (cur_di > 0) ? cur_di - 1 : dk_count - 1;
                s->selected_index = dk_indices[new_di];
                // Auto scroll into view
                if (new_di < s->scroll_offset) {
                    s->scroll_offset = new_di;
                } else if (new_di >= s->scroll_offset + EXPLORER_MAX_VISIBLE) {
                    s->scroll_offset = new_di - (EXPLORER_MAX_VISIBLE - 1);
                }
                if (s->scroll_offset < 0) s->scroll_offset = 0;
                maxp_draw_active_instance();
                play_sound(550); sleep(30); no_sound();
            }
            return 1;
        }

        if (scan_code == 0x50 || ascii_char == 'D') {
            if (dk_count > 0) {
                int new_di = (cur_di >= 0 && cur_di < dk_count - 1) ? cur_di + 1 : 0;
                s->selected_index = dk_indices[new_di];
                // Auto scroll into view
                if (new_di < s->scroll_offset) {
                    s->scroll_offset = new_di;
                } else if (new_di >= s->scroll_offset + EXPLORER_MAX_VISIBLE) {
                    s->scroll_offset = new_di - (EXPLORER_MAX_VISIBLE - 1);
                }
                if (s->scroll_offset < 0) s->scroll_offset = 0;
                maxp_draw_active_instance();
                play_sound(550); sleep(30); no_sound();
            }
            return 1;
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void explorer_init(void) {
    explorer_open = 0;
    explorer_instance_init(&primary_explorer_state);
}

void explorer_open_window(void) {
    explorer_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_EXPLORER, "Explorer", 0);
}

void explorer_close_window(void) {
    explorer_open = 0;
    maxp_close_app(MAXP_APP_EXPLORER);
}

void explorer_draw(void) {
    explorer_instance_draw(&primary_explorer_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45);
}

int explorer_handle_click(int mouse_x, int mouse_y) {
    int res = explorer_instance_click(&primary_explorer_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45, mouse_x, mouse_y);
    if (res == -1) {
        explorer_close_window();
        return 1;
    }
    return res;
}

int explorer_handle_key(char ascii_char, unsigned char scan_code) {
    int res = explorer_instance_key(&primary_explorer_state, ascii_char, scan_code);
    if (res == -1) {
        explorer_close_window();
        return 1;
    }
    return res;
}

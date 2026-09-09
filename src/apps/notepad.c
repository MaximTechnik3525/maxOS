#include "notepad.h"
#include "maxfs.h"
#include "maxp.h"
#include "kernel.h"

extern int km_mode;

int notepad_open = 0;
char ftext[1024] = {0};
int textid = 0;
char fname_input[32] = "note.txt";
int fname_len = 8;

static notepad_state_t primary_notepad_state;

static const unsigned short text_colors[] = {
    0x0000, // Black
    0x041F, // Classic Blue
    0x05E5, // Matrix Green
    0xF800, // Crimson Red
    0xFD20, // Amber
    0x981F  // Purple
};

static const char* color_names[] = {
    "Black", "Blue", "Green", "Red", "Amber", "Purple"
};

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    int len = 0;
    while (label[len] != '\0') len++;
    int tx = bx + (bw - (len * 9)) / 2;
    int ty = by + (bh - 8) / 2;
    if (tx < bx + 2) tx = bx + 2;
    if (ty < by + 1) ty = by + 1;
    print_string((char*)label, tx, ty, text_col);
}

static void notepad_set_status_s(notepad_state_t* s, const char* msg) {
    int i = 0;
    while (msg[i] != '\0' && i < 62) {
        s->np_status[i] = msg[i];
        i++;
    }
    s->np_status[i] = '\0';
}

void notepad_instance_init(notepad_state_t* s, const char* initial_file) {
    s->textid = 0;
    s->current_color_idx = 0;
    s->focus_mode = 0;
    s->file_picker_open = 0;
    s->picker_selected = 0;
    for (int i = 0; i < 2048; i++) s->ftext[i] = '\0';

    if (initial_file && initial_file[0] != '\0') {
        int slot = maxfs_find_file(initial_file);
        if (slot >= 0) {
            struct VirtualFile* vf = maxfs_get_file(slot);
            if (vf && vf->exists) {
                s->fname_len = 0;
                while (vf->name[s->fname_len] != '\0' && s->fname_len < 31) {
                    s->fname_input[s->fname_len] = vf->name[s->fname_len];
                    s->fname_len++;
                }
                s->fname_input[s->fname_len] = '\0';

                s->textid = 0;
                while (vf->content[s->textid] != '\0' && s->textid < 2040) {
                    s->ftext[s->textid] = vf->content[s->textid];
                    s->textid++;
                }
                s->ftext[s->textid] = '\0';

                char msg[64] = "Loaded: ";
                int p = 8;
                for (int i = 0; s->fname_input[i] != '\0' && p < 50; i++) msg[p++] = s->fname_input[i];
                msg[p] = '\0';
                notepad_set_status_s(s, msg);
                return;
            }
        }
    }

    s->fname_input[0] = 'n'; s->fname_input[1] = 'o'; s->fname_input[2] = 't'; s->fname_input[3] = 'e';
    s->fname_input[4] = '.'; s->fname_input[5] = 't'; s->fname_input[6] = 'x'; s->fname_input[7] = 't';
    s->fname_input[8] = '\0';
    s->fname_len = 8;
    notepad_set_status_s(s, "Ready | Click [Save] or [Open]");
}

static void notepad_instance_save(notepad_state_t* s) {
    if (s->fname_len == 0) {
        s->fname_input[0] = 'n'; s->fname_input[1] = 'o'; s->fname_input[2] = 't'; s->fname_input[3] = 'e';
        s->fname_input[4] = '.'; s->fname_input[5] = 't'; s->fname_input[6] = 'x'; s->fname_input[7] = 't';
        s->fname_input[8] = '\0';
        s->fname_len = 8;
    }

    int res = maxfs_write_file(s->fname_input, s->ftext, s->textid);
    if (res >= 0) {
        char msg[64] = "Saved: ";
        int p = 7;
        for (int i = 0; s->fname_input[i] != '\0' && p < 50; i++) msg[p++] = s->fname_input[i];
        msg[p++] = ' '; msg[p++] = 't'; msg[p++] = 'o'; msg[p++] = ' ';
        msg[p++] = 'd'; msg[p++] = 'i'; msg[p++] = 's'; msg[p++] = 'k';
        msg[p++] = '!'; msg[p] = '\0';
        notepad_set_status_s(s, msg);
        play_sound(700); sleep(50); play_sound(1100); sleep(70); no_sound();
    } else {
        notepad_set_status_s(s, "Error saving to maxFS disk!");
        play_sound(200); sleep(150); no_sound();
    }
}

static void notepad_instance_new_file(notepad_state_t* s) {
    for (int i = 0; i < 2048; i++) s->ftext[i] = '\0';
    s->textid = 0;
    s->fname_input[0] = 'n'; s->fname_input[1] = 'e'; s->fname_input[2] = 'w'; s->fname_input[3] = '.';
    s->fname_input[4] = 't'; s->fname_input[5] = 'x'; s->fname_input[6] = 't'; s->fname_input[7] = '\0';
    s->fname_len = 7;
    s->focus_mode = 0;
    s->file_picker_open = 0;
    notepad_set_status_s(s, "New empty note created.");
    play_sound(500); sleep(60); no_sound();
}

static void notepad_instance_load_file(notepad_state_t* s, const char* name) {
    int slot = maxfs_find_file(name);
    if (slot < 0) {
        notepad_set_status_s(s, "File not found!");
        play_sound(200); sleep(100); no_sound();
        return;
    }

    struct VirtualFile* vf = maxfs_get_file(slot);
    if (!vf || !vf->exists) return;

    s->fname_len = 0;
    while (vf->name[s->fname_len] != '\0' && s->fname_len < 31) {
        s->fname_input[s->fname_len] = vf->name[s->fname_len];
        s->fname_len++;
    }
    s->fname_input[s->fname_len] = '\0';

    s->textid = 0;
    while (vf->content[s->textid] != '\0' && s->textid < 2040) {
        s->ftext[s->textid] = vf->content[s->textid];
        s->textid++;
    }
    s->ftext[s->textid] = '\0';

    char msg[64] = "Loaded: ";
    int p = 8;
    for (int i = 0; s->fname_input[i] != '\0' && p < 50; i++) msg[p++] = s->fname_input[i];
    msg[p] = '\0';
    notepad_set_status_s(s, msg);
    s->focus_mode = 0;
    s->file_picker_open = 0;
    play_sound(600); sleep(60); play_sound(900); sleep(60); no_sound();
}

static void notepad_instance_render_toolbar(notepad_state_t* s, int np_x, int np_y, int np_w) {
    // Row 1: Actions Toolbar
    draw_rect(np_x + 2, np_y + 22, np_w - 4, 25, 0xDF17);
    draw_rect(np_x + 2, np_y + 46, np_w - 4, 1, 0x9CD3);

    // Label: "File:"
    print_string("File:", np_x + 8, np_y + 28, 0x0000);

    // Filename input box
    int fbox_x = np_x + 50;
    int fbox_y = np_y + 25;
    int fbox_w = 135;
    int fbox_h = 19;
    draw_rect(fbox_x, fbox_y, fbox_w, fbox_h, 0x0000);
    unsigned short fbox_fill = (s->focus_mode == 1) ? 0xFFE0 : 0xFFFF;
    draw_rect(fbox_x + 1, fbox_y + 1, fbox_w - 2, fbox_h - 2, fbox_fill);
    print_string(s->fname_input, fbox_x + 4, fbox_y + 4, 0x0000);
    if (s->focus_mode == 1) {
        print_string("_", fbox_x + 4 + s->fname_len * 9, fbox_y + 4, 0x0000);
    }

    // [Save] button (Green)
    draw_ui_btn(np_x + 192, np_y + 25, 48, 19, "Save", 0x3DF2, 0x0000);

    // [Open] button (Blue)
    draw_ui_btn(np_x + 246, np_y + 25, 48, 19, "Open", 0x24EE, 0xFFFF);

    // [New] button (Gray)
    draw_ui_btn(np_x + 300, np_y + 25, 44, 19, "New",  0xC618, 0x0000);

    // [Col] button
    draw_ui_btn(np_x + 350, np_y + 25, 44, 19, "Col",  0x7BEF, 0x0000);

    // Color swatches (6 clickable chips)
    for (int i = 0; i < 6; i++) {
        int cx = np_x + 402 + (i * 17);
        int cy = np_y + 26;
        draw_rect(cx, cy, 14, 17, 0x0000);
        if (i == s->current_color_idx) {
            draw_rect(cx + 1, cy + 1, 12, 1, 0xFFFF);
            draw_rect(cx + 1, cy + 1, 1, 15, 0xFFFF);
            draw_rect(cx + 12, cy + 1, 1, 15, 0xFFFF);
            draw_rect(cx + 1, cy + 15, 12, 1, 0xFFFF);
            draw_rect(cx + 2, cy + 2, 10, 13, text_colors[i]);
        } else {
            draw_rect(cx + 1, cy + 1, 12, 15, text_colors[i]);
        }
    }


    // Row 2: Status & Details
    draw_rect(np_x + 2, np_y + 47, np_w - 4, 19, 0xE71C);
    print_string("Status:", np_x + 8, np_y + 51, 0x0000);
    print_string(s->np_status, np_x + 65, np_y + 51, 0x001F);

    char col_lbl[24];
    col_lbl[0] = 'C'; col_lbl[1] = 'o'; col_lbl[2] = 'l'; col_lbl[3] = 'o'; col_lbl[4] = 'r';
    col_lbl[5] = ':'; col_lbl[6] = ' ';
    int cp = 7;
    const char* cname = color_names[s->current_color_idx];
    for (int i = 0; cname[i] != '\0'; i++) col_lbl[cp++] = cname[i];
    col_lbl[cp] = '\0';
    print_string(col_lbl, np_x + np_w - 130, np_y + 51, text_colors[s->current_color_idx]);
}

static void notepad_instance_draw_text_box(notepad_state_t* s, int np_x, int np_y, int np_w, int np_h) {
    int area_x = np_x + 10;
    int area_y = np_y + 68;
    int area_w = np_w - 20;
    int area_h = np_h - 96;

    draw_rect(area_x, area_y, area_w, area_h, 0xFFFF);
    draw_rect(area_x, area_y, area_w, 1, 0x7BEF);
    draw_rect(area_x, area_y, 1, area_h, 0x7BEF);
    draw_rect(area_x, area_y + area_h - 1, area_w, 1, 0x7BEF);
    draw_rect(area_x + area_w - 1, area_y, 1, area_h, 0x7BEF);

    int start_x = area_x + 8;
    int cur_x = start_x;
    int cur_y = area_y + 8;
    int max_chars_per_line = (area_w - 20) / 9;
    if (max_chars_per_line < 10) max_chars_per_line = 10;
    int chars_in_line = 0;

    unsigned short active_color = text_colors[s->current_color_idx];

    for (int i = 0; i < s->textid; i++) {
        char ch = s->ftext[i];
        if (ch == '\n') {
            cur_x = start_x;
            cur_y += 14;
            chars_in_line = 0;
            if (cur_y + 12 >= area_y + area_h) break;
            continue;
        }

        if (chars_in_line >= max_chars_per_line) {
            cur_x = start_x;
            cur_y += 14;
            chars_in_line = 0;
            if (cur_y + 12 >= area_y + area_h) break;
        }

        draw_char(ch, cur_x, cur_y, active_color);
        cur_x += 9;
        chars_in_line++;
    }

    if (s->focus_mode == 0 && cur_y + 12 < area_y + area_h) {
        print_string("_", cur_x, cur_y, active_color);
    }
}

static void notepad_instance_draw_file_picker(notepad_state_t* s, int np_x, int np_y, int np_w, int np_h) {
    int dlg_w = 420;
    int dlg_h = 320;
    int dlg_x = np_x + (np_w - dlg_w) / 2;
    int dlg_y = np_y + (np_h - dlg_h) / 2;

    draw_rect(dlg_x + 4, dlg_y + 4, dlg_w, dlg_h, 0x39E7); // Drop shadow
    draw_rect(dlg_x, dlg_y, dlg_w, dlg_h, 0x0000);
    draw_rect(dlg_x + 1, dlg_y + 1, dlg_w - 2, dlg_h - 2, 0xEF59);

    draw_rect(dlg_x + 2, dlg_y + 2, dlg_w - 4, 22, 0x11EB);
    print_string("Open File from maxFS 2.0", dlg_x + 10, dlg_y + 6, 0xFFFF);

    print_string("Click a file to open in Notepad:", dlg_x + 14, dlg_y + 32, 0x0000);

    draw_rect(dlg_x + 12, dlg_y + 48, dlg_w - 24, dlg_h - 90, 0xFFFF);
    draw_rect(dlg_x + 12, dlg_y + 48, dlg_w - 24, 1, 0x7BEF);
    draw_rect(dlg_x + 12, dlg_y + 48, 1, dlg_h - 90, 0x7BEF);
    draw_rect(dlg_x + 12, dlg_y + dlg_h - 43, dlg_w - 24, 1, 0x7BEF);
    draw_rect(dlg_x + dlg_w - 13, dlg_y + 48, 1, dlg_h - 90, 0x7BEF);

    int row_y = dlg_y + 54;
    int file_count = 0;
    for (int i = 0; i < MAXFS_MAX_FILES && file_count < 8; i++) {
        struct VirtualFile* vf = maxfs_get_file(i);
        if (vf && vf->exists) {
            if (s->picker_selected == file_count) {
                draw_rect(dlg_x + 16, row_y, dlg_w - 32, 20, 0x861F);
                print_string(">", dlg_x + 8, row_y + 4, 0x0000);
            } else {
                draw_rect(dlg_x + 16, row_y, dlg_w - 32, 20, 0xDF17);
            }
            draw_rect(dlg_x + 16, row_y, dlg_w - 32, 1, 0x9CD3);
            draw_rect(dlg_x + 16, row_y + 19, dlg_w - 32, 1, 0x9CD3);

            print_string("[TXT]", dlg_x + 20, row_y + 4, 0x24EE);
            print_string(vf->name, dlg_x + 70, row_y + 4, 0x0000);

            char sz_str[16];
            int_str(vf->size, sz_str);
            int p = 0; while (sz_str[p] != '\0') p++;
            sz_str[p++] = ' '; sz_str[p++] = 'B'; sz_str[p] = '\0';
            print_string(sz_str, dlg_x + 280, row_y + 4, 0x0320);

            row_y += 24;
            file_count++;
        }
    }

    if (file_count == 0) {
        print_string("No files found on disk.", dlg_x + 30, dlg_y + 80, 0xF800);
        print_string("Type text and click [Save] to create one.", dlg_x + 30, dlg_y + 100, 0x0000);
    }

    draw_ui_btn(dlg_x + dlg_w - 85, dlg_y + dlg_h - 34, 72, 22, "Cancel", 0xF9A6, 0x0000);
}

void notepad_instance_draw(notepad_state_t* s, int np_x, int np_y, int np_w, int np_h) {
    prev_cursor();

    // Window frame
    draw_rect(np_x, np_y, np_w, np_h, 0x0000);
    draw_rect(np_x + 1, np_y + 1, np_w - 2, np_h - 2, 0xEF59);

    // Titlebar
    draw_rect(np_x + 2, np_y + 2, np_w - 4, 20, 0x24EE);
    char title[64];
    title[0] = 'm'; title[1] = 'a'; title[2] = 'x'; title[3] = 'O'; title[4] = 'S';
    title[5] = ' '; title[6] = 'N'; title[7] = 'o'; title[8] = 't'; title[9] = 'e';
    title[10] = 'p'; title[11] = 'a'; title[12] = 'd'; title[13] = ' ';
    title[14] = '3'; title[15] = '.'; title[16] = '2'; title[17] = ' ';
    title[18] = '-'; title[19] = ' ';
    title[20] = '[';
    int tp = 21;
    for (int i = 0; s->fname_input[i] != '\0' && tp < 55; i++) title[tp++] = s->fname_input[i];
    title[tp++] = ']';
    title[tp] = '\0';
    print_string(title, np_x + 8, np_y + 6, 0xFFFF);

    // [X] Close button (Titlebar)
    draw_ui_btn(np_x + np_w - 22, np_y + 4, 18, 16, "X", 0xF800, 0xFFFF);

    // Toolbar
    notepad_instance_render_toolbar(s, np_x, np_y, np_w);

    // Footer
    draw_rect(np_x + 2, np_y + np_h - 24, np_w - 4, 22, 0xDF17);
    draw_rect(np_x + 2, np_y + np_h - 24, np_w - 4, 1, 0x9CD3);
    print_string("Click File: to rename | [Open] to pick file | [Col] to change color | Esc exit", np_x + 10, np_y + np_h - 18, 0x0000);

    // Text area
    notepad_instance_draw_text_box(s, np_x, np_y, np_w, np_h);

    // If file picker modal is open, overlay it
    if (s->file_picker_open) {
        notepad_instance_draw_file_picker(s, np_x, np_y, np_w, np_h);
    }

    draw_cursor(pos_x, pos_y);
}

int notepad_instance_click(notepad_state_t* s, int np_x, int np_y, int np_w, int np_h, int mouse_x, int mouse_y) {
    if (s->file_picker_open) {
        int dlg_w = 420;
        int dlg_h = 320;
        int dlg_x = np_x + (np_w - dlg_w) / 2;
        int dlg_y = np_y + (np_h - dlg_h) / 2;

        // Check [Cancel] button
        if (mouse_x >= dlg_x + dlg_w - 85 && mouse_x <= dlg_x + dlg_w - 13 &&
            mouse_y >= dlg_y + dlg_h - 34 && mouse_y <= dlg_y + dlg_h - 12) {
            s->file_picker_open = 0;
            draw_window();
            play_sound(400); sleep(50); no_sound();
            return 1;
        }

        // Check clicking on a file item in the list
        int row_y = dlg_y + 54;
        int file_count = 0;
        for (int i = 0; i < MAXFS_MAX_FILES && file_count < 8; i++) {
            struct VirtualFile* vf = maxfs_get_file(i);
            if (vf && vf->exists) {
                if (mouse_x >= dlg_x + 16 && mouse_x <= dlg_x + dlg_w - 16 &&
                    mouse_y >= row_y && mouse_y <= row_y + 20) {
                    s->file_picker_open = 0;
                    notepad_instance_load_file(s, vf->name);
                    draw_window();
                    return 1;
                }
                row_y += 24;
                file_count++;
            }
        }
        return 1;
    }

    // Check Filename input box: np_x + 50 .. np_x + 185, np_y + 25 .. np_y + 44
    if (mouse_x >= np_x + 50 && mouse_x <= np_x + 185 && mouse_y >= np_y + 25 && mouse_y <= np_y + 44) {
        s->focus_mode = 1;
        draw_window();
        play_sound(600); sleep(40); no_sound();
        return 1;
    }

    // Check [Save] button: np_x + 192 .. np_x + 240
    if (mouse_x >= np_x + 192 && mouse_x <= np_x + 240 && mouse_y >= np_y + 25 && mouse_y <= np_y + 44) {
        notepad_instance_save(s);
        draw_window();
        return 1;
    }

    // Check [Open] button: np_x + 246 .. np_x + 294
    if (mouse_x >= np_x + 246 && mouse_x <= np_x + 294 && mouse_y >= np_y + 25 && mouse_y <= np_y + 44) {
        s->file_picker_open = 1;
        s->picker_selected = 0;
        draw_window();
        play_sound(600); sleep(60); no_sound();
        return 1;
    }

    // Check [New] button: np_x + 300 .. np_x + 344
    if (mouse_x >= np_x + 300 && mouse_x <= np_x + 344 && mouse_y >= np_y + 25 && mouse_y <= np_y + 44) {
        notepad_instance_new_file(s);
        draw_window();
        return 1;
    }

    // Check [Col] button: np_x + 350 .. np_x + 394
    if (mouse_x >= np_x + 350 && mouse_x <= np_x + 394 && mouse_y >= np_y + 25 && mouse_y <= np_y + 44) {
        s->current_color_idx = (s->current_color_idx + 1) % 6;
        draw_window();
        play_sound(750); sleep(40); no_sound();
        return 1;
    }

    // Check Color Swatches: np_x + 402 + i*17 .. 14 wide
    for (int i = 0; i < 6; i++) {
        int cx = np_x + 402 + (i * 17);
        if (mouse_x >= cx && mouse_x <= cx + 14 && mouse_y >= np_y + 26 && mouse_y <= np_y + 43) {
            s->current_color_idx = i;
            draw_window();
            play_sound(800 + i * 50); sleep(35); no_sound();
            return 1;
        }
    }

    // Check [X] Close button (Titlebar)
    if (mouse_x >= np_x + np_w - 26 && mouse_x <= np_x + np_w && mouse_y >= np_y && mouse_y <= np_y + 24) {
        return -1; // Request close
    }

    // Click inside text area
    int area_x = np_x + 10;
    int area_y = np_y + 68;
    int area_w = np_w - 20;
    int area_h = np_h - 96;
    if (mouse_x >= area_x && mouse_x <= area_x + area_w && mouse_y >= area_y && mouse_y <= area_y + area_h) {
        if (s->focus_mode != 0) {
            s->focus_mode = 0;
            draw_window();
        }
        return 1;
    }

    // Swallow clicks inside window frame
    if (mouse_x >= np_x && mouse_x <= np_x + np_w && mouse_y >= np_y && mouse_y <= np_y + np_h) {
        return 1;
    }

    return 0;
}

int notepad_instance_key(notepad_state_t* s, char ascii_char, unsigned char scan_code) {
    // F2 (0x3C) or Escape (0x01)
    if (scan_code == 0x01 || scan_code == 0x3C) {
        if (s->file_picker_open) {
            s->file_picker_open = 0;
            draw_window();
            return 1;
        }
        return -1; // Request close
    }

    if (s->file_picker_open) {
        int total = 0;
        int file_indices[MAXFS_MAX_FILES];
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            struct VirtualFile* vf = maxfs_get_file(i);
            if (vf && vf->exists) {
                file_indices[total++] = i;
            }
        }
        if (total > 8) total = 8;

        // Up arrow
        if (scan_code == 0x48 || ascii_char == 'U') {
            if (s->picker_selected > 0) {
                s->picker_selected--;
                draw_window();
                play_sound(550); sleep(25); no_sound();
            }
            return 1;
        }

        // Down arrow
        if (scan_code == 0x50 || ascii_char == 'D') {
            if (s->picker_selected < total - 1) {
                s->picker_selected++;
                draw_window();
                play_sound(550); sleep(25); no_sound();
            }
            return 1;
        }

        // Enter: select file
        if (scan_code == 0x1C || ascii_char == '\n') {
            if (total > 0 && s->picker_selected >= 0 && s->picker_selected < total) {
                s->file_picker_open = 0;
                struct VirtualFile* vf = maxfs_get_file(file_indices[s->picker_selected]);
                if (vf) notepad_instance_load_file(s, vf->name);
                draw_window();
            }
            return 1;
        }

        return 1;
    }

    // Tab (0x0F) toggles focus
    if (scan_code == 0x0F || ascii_char == '\t') {
        s->focus_mode = 1 - s->focus_mode;
        draw_window();
        play_sound(600); sleep(30); no_sound();
        return 1;
    }

    // F3 (0x3D) quick save
    if (scan_code == 0x3D && km_mode == 0) {
        notepad_instance_save(s);
        draw_window();
        return 1;
    }

    // F4 (0x3E) quick open file picker
    if (scan_code == 0x3E && km_mode == 0) {
        s->file_picker_open = 1;
        s->picker_selected = 0;
        draw_window();
        return 1;
    }

    // F7 (0x41) or F8 (0x42) quick cycle color
    if ((scan_code == 0x41 || scan_code == 0x42) && km_mode == 0) {
        s->current_color_idx = (s->current_color_idx + 1) % 6;
        draw_window();
        play_sound(750 + s->current_color_idx * 50); sleep(30); no_sound();
        return 1;
    }

    if (ascii_char == 'T' || ascii_char == 'G') return 0;
    if (km_mode == 1 && (ascii_char == 'U' || ascii_char == 'D' || ascii_char == 'L' || ascii_char == 'R' || ascii_char == 'e')) {
        return 0;
    }

    // Focus Mode 1: Editing Filename
    if (s->focus_mode == 1) {
        if (scan_code == 0x0E || ascii_char == 'B') {
            if (s->fname_len > 0) {
                s->fname_len--;
                s->fname_input[s->fname_len] = '\0';
                draw_window();
            }
            return 1;
        }

        if (scan_code == 0x1C || ascii_char == '\n') {
            s->focus_mode = 0;
            notepad_instance_save(s);
            draw_window();
            return 1;
        }

        if ((ascii_char >= 'a' && ascii_char <= 'z') ||
            (ascii_char >= '0' && ascii_char <= '9') ||
            ascii_char == '.' || ascii_char == '_' || ascii_char == '-') {
            if (s->fname_len < 30) {
                s->fname_input[s->fname_len++] = ascii_char;
                s->fname_input[s->fname_len] = '\0';
                draw_window();
            }
            return 1;
        }
        return 1;
    }

    // Focus Mode 0: Editing Text Content
    if (scan_code == 0x0E || ascii_char == 'B') {
        if (s->textid > 0) {
            s->textid--;
            s->ftext[s->textid] = '\0';
            draw_window();
        }
        return 1;
    }

    if (scan_code == 0x1C || ascii_char == '\n') {
        if (s->textid < 2040) {
            s->ftext[s->textid++] = '\n';
            s->ftext[s->textid] = '\0';
            draw_window();
        }
        return 1;
    }

    if ((ascii_char >= 'a' && ascii_char <= 'z') ||
        (ascii_char >= 'A' && ascii_char <= 'Z') ||
        (ascii_char >= '0' && ascii_char <= '9') ||
        ascii_char == ' ' || ascii_char == '.' || ascii_char == ',' ||
        ascii_char == '/' || ascii_char == '-' || ascii_char == '=' ||
        ascii_char == ':' || ascii_char == ';' || ascii_char == '!' ||
        ascii_char == '?' || ascii_char == '(' || ascii_char == ')') {
        if (s->textid < 2040) {
            s->ftext[s->textid++] = ascii_char;
            s->ftext[s->textid] = '\0';
            draw_window();
        }
        return 1;
    }

    return 1;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void notepad_init(void) {
    notepad_open = 0;
    notepad_instance_init(&primary_notepad_state, 0);
}

void notepad_open_window(void) {
    notepad_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", 0);
}

void notepad_open_file_by_name(const char* name) {
    notepad_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_NOTEPAD, "Notepad", name);
}

void notepad_close_window(void) {
    notepad_open = 0;
    maxp_close_app(MAXP_APP_NOTEPAD);
}

void notepad_draw(void) {
    notepad_instance_draw(&primary_notepad_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45);
}

void notepad_draw_text(void) {
    notepad_instance_draw_text_box(&primary_notepad_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45);
}

int notepad_handle_click(int mouse_x, int mouse_y) {
    int res = notepad_instance_click(&primary_notepad_state, win_x + 15, win_y + 35, win_w - 30, win_h - 45, mouse_x, mouse_y);
    if (res == -1) {
        notepad_close_window();
        return 1;
    }
    return res;
}

int notepad_handle_key(char ascii_char, unsigned char scan_code) {
    int res = notepad_instance_key(&primary_notepad_state, ascii_char, scan_code);
    if (res == -1) {
        notepad_close_window();
        return 1;
    }
    return res;
}

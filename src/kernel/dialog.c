#include "dialog.h"
#include "kernel.h"
#include "string.h"

void dialog_init(dialog_t* dlg, int type, const char* title, const char* message) {
    if (!dlg) return;
    memset(dlg, 0, sizeof(dialog_t));
    dlg->dialog_type = type;
    if (title) {
        strncpy(dlg->title, title, sizeof(dlg->title) - 1);
        dlg->title[sizeof(dlg->title) - 1] = '\0';
    }
    if (message) {
        strncpy(dlg->message, message, sizeof(dlg->message) - 1);
        dlg->message[sizeof(dlg->message) - 1] = '\0';
    }
    strcpy(dlg->ok_label, "OK");
    strcpy(dlg->cancel_label, "Cancel");
    dlg->w = 380;
    dlg->h = 160;
    dlg->focused_field = 0;
    dlg->active = 0;
    dlg->result = DIALOG_RES_NONE;
}

void dialog_set_geometry(dialog_t* dlg, int x, int y, int w, int h) {
    if (!dlg) return;
    dlg->x = x;
    dlg->y = y;
    if (w > 0) dlg->w = w;
    if (h > 0) dlg->h = h;
}

void dialog_center(dialog_t* dlg, int parent_x, int parent_y, int parent_w, int parent_h) {
    if (!dlg) return;
    int min_h = 80 + dlg->field_count * 44 + 40;
    if (dlg->h < min_h) dlg->h = min_h;
    if (dlg->w < 380) dlg->w = 380;

    dlg->x = parent_x + (parent_w - dlg->w) / 2;
    dlg->y = parent_y + (parent_h - dlg->h) / 2;
    if (dlg->x < 5) dlg->x = 5;
    if (dlg->y < 25) dlg->y = 25;
}

int dialog_add_field(dialog_t* dlg, const char* label, const char* default_val, int max_len) {
    if (!dlg || dlg->field_count >= DIALOG_MAX_FIELDS) return -1;
    int idx = dlg->field_count++;
    dialog_field_t* f = &dlg->fields[idx];
    memset(f, 0, sizeof(dialog_field_t));

    if (label) {
        strncpy(f->label, label, sizeof(f->label) - 1);
        f->label[sizeof(f->label) - 1] = '\0';
    }
    if (default_val) {
        strncpy(f->value, default_val, sizeof(f->value) - 1);
        f->value[sizeof(f->value) - 1] = '\0';
    }
    f->len = (int)strlen(f->value);
    f->cursor = f->len;
    f->max_len = (max_len > 0 && max_len < DIALOG_FIELD_VALUE_LEN) ? max_len : (DIALOG_FIELD_VALUE_LEN - 1);

    int min_h = 80 + dlg->field_count * 44 + 40;
    if (dlg->h < min_h) dlg->h = min_h;
    return idx;
}

void dialog_set_buttons(dialog_t* dlg, const char* ok_label, const char* cancel_label) {
    if (!dlg) return;
    if (ok_label) {
        strncpy(dlg->ok_label, ok_label, sizeof(dlg->ok_label) - 1);
        dlg->ok_label[sizeof(dlg->ok_label) - 1] = '\0';
    }
    if (cancel_label) {
        strncpy(dlg->cancel_label, cancel_label, sizeof(dlg->cancel_label) - 1);
        dlg->cancel_label[sizeof(dlg->cancel_label) - 1] = '\0';
    }
}

void dialog_show(dialog_t* dlg) {
    if (!dlg) return;
    int min_h = 80 + dlg->field_count * 44 + 40;
    if (dlg->h < min_h) dlg->h = min_h;

    dlg->ok_btn_w = 75;
    dlg->ok_btn_h = 22;
    dlg->cancel_btn_w = 75;
    dlg->cancel_btn_h = 22;

    dlg->cancel_btn_x = dlg->x + dlg->w - 88;
    dlg->cancel_btn_y = dlg->y + dlg->h - 32;

    dlg->ok_btn_x = dlg->cancel_btn_x - 85;
    dlg->ok_btn_y = dlg->y + dlg->h - 32;

    dlg->active = 1;
    dlg->result = DIALOG_RES_NONE;
    dlg->focused_field = 0;
    if (dlg->field_count > 0) {
        dlg->fields[0].cursor = dlg->fields[0].len;
    }
}

void dialog_close(dialog_t* dlg) {
    if (!dlg) return;
    dlg->active = 0;
}

void dialog_draw(dialog_t* dlg) {
    if (!dlg || !dlg->active) return;

    int dx = dlg->x;
    int dy = dlg->y;
    int dw = dlg->w;
    int dh = dlg->h;

    // Drop shadow
    draw_rect(dx + 5, dy + 5, dw, dh, 0x39E7);

    // Window frame
    draw_rect(dx, dy, dw, dh, 0x0000);
    draw_rect(dx + 1, dy + 1, dw - 2, dh - 2, 0xEF59);

    // 3D bevel inside
    draw_rect(dx + 1, dy + 1, dw - 2, 1, 0xFFFF);
    draw_rect(dx + 1, dy + 1, 1, dh - 2, 0xFFFF);
    draw_rect(dx + dw - 2, dy + 1, 1, dh - 2, 0x7BEF);
    draw_rect(dx + 1, dy + dh - 2, dw - 2, 1, 0x7BEF);

    // Titlebar
    draw_rect(dx + 2, dy + 2, dw - 4, 20, 0x11EB);
    print_string((char*)dlg->title, dx + 8, dy + 6, 0xFFFF);

    // [X] Close button
    draw_ui_btn(dx + dw - 22, dy + 4, 18, 16, "X", 0xF800, 0xFFFF);

    // Optional message / prompt
    int msg_y = dy + 28;
    if (dlg->message[0] != '\0') {
        print_string((char*)dlg->message, dx + 14, msg_y, 0x0000);
        msg_y += 18;
    }

    // Fields
    int fy = msg_y;
    for (int i = 0; i < dlg->field_count; i++) {
        dialog_field_t* f = &dlg->fields[i];

        // Label
        print_string((char*)f->label, dx + 14, fy, 0x0000);

        // Input box
        int bx = dx + 14;
        int by = fy + 15;
        int bw = dw - 28;
        int bh = 22;

        draw_rect(bx, by, bw, bh, 0x0000);
        draw_rect(bx + 1, by + 1, bw - 2, 1, 0x7BEF);
        draw_rect(bx + 1, by + 1, 1, bh - 2, 0x7BEF);

        unsigned short fill = (dlg->focused_field == i) ? 0xFFFF : 0xF7BE;
        draw_rect(bx + 2, by + 2, bw - 3, bh - 3, fill);

        // Field text
        print_string((char*)f->value, bx + 6, by + 5, 0x0000);

        // Cursor
        if (dlg->focused_field == i) {
            int cur_x = bx + 6 + f->cursor * 9;
            if (cur_x + 9 < bx + bw - 4) {
                print_string("_", cur_x, by + 5, 0x0000);
            }
        }

        fy += 44;
    }

    // Action buttons
    draw_ui_btn(dlg->ok_btn_x, dlg->ok_btn_y, dlg->ok_btn_w, dlg->ok_btn_h, dlg->ok_label, 0x3DF2, 0x0000);
    draw_ui_btn(dlg->cancel_btn_x, dlg->cancel_btn_y, dlg->cancel_btn_w, dlg->cancel_btn_h, dlg->cancel_label, 0xC618, 0x0000);
}

int dialog_handle_click(dialog_t* dlg, int mouse_x, int mouse_y) {
    if (!dlg || !dlg->active) return 0;

    int dx = dlg->x;
    int dy = dlg->y;
    int dw = dlg->w;

    // 1. [X] button
    if (mouse_x >= dx + dw - 24 && mouse_x <= dx + dw && mouse_y >= dy && mouse_y <= dy + 22) {
        ui_btn_click_effect(dx + dw - 22, dy + 4, 18, 16, "X", 0xF800, 0xFFFF);
        dlg->result = DIALOG_RES_CANCEL;
        dlg->active = 0;
        play_sound(400); sleep(40); no_sound();
        return 1;
    }

    // 2. [OK] button
    if (mouse_x >= dlg->ok_btn_x && mouse_x <= dlg->ok_btn_x + dlg->ok_btn_w &&
        mouse_y >= dlg->ok_btn_y && mouse_y <= dlg->ok_btn_y + dlg->ok_btn_h) {
        ui_btn_click_effect(dlg->ok_btn_x, dlg->ok_btn_y, dlg->ok_btn_w, dlg->ok_btn_h, dlg->ok_label, 0x3DF2, 0x0000);
        dlg->result = DIALOG_RES_OK;
        dlg->active = 0;
        play_sound(800); sleep(40); no_sound();
        return 1;
    }

    // 3. [Cancel] button
    if (mouse_x >= dlg->cancel_btn_x && mouse_x <= dlg->cancel_btn_x + dlg->cancel_btn_w &&
        mouse_y >= dlg->cancel_btn_y && mouse_y <= dlg->cancel_btn_y + dlg->cancel_btn_h) {
        ui_btn_click_effect(dlg->cancel_btn_x, dlg->cancel_btn_y, dlg->cancel_btn_w, dlg->cancel_btn_h, dlg->cancel_label, 0xC618, 0x0000);
        dlg->result = DIALOG_RES_CANCEL;
        dlg->active = 0;
        play_sound(400); sleep(40); no_sound();
        return 1;
    }

    // 4. Click inside input boxes
    int msg_y = dy + 28;
    if (dlg->message[0] != '\0') msg_y += 18;
    int fy = msg_y;
    for (int i = 0; i < dlg->field_count; i++) {
        int bx = dx + 14;
        int by = fy + 15;
        int bw = dw - 28;
        int bh = 22;

        if (mouse_x >= bx && mouse_x <= bx + bw && mouse_y >= by && mouse_y <= by + bh) {
            dlg->focused_field = i;
            dialog_field_t* f = &dlg->fields[i];
            int click_offset = (mouse_x - (bx + 6) + 4) / 9;
            if (click_offset < 0) click_offset = 0;
            if (click_offset > f->len) click_offset = f->len;
            f->cursor = click_offset;
            play_sound(600); sleep(20); no_sound();
            return 1;
        }
        fy += 44;
    }

    // Modal: consume all clicks within dialog or window bounds
    return 1;
}

int dialog_handle_key(dialog_t* dlg, char ascii_char, unsigned char scan_code) {
    if (!dlg || !dlg->active) return 0;

    // Enter: Confirm / Submit
    if (scan_code == 0x1C || ascii_char == '\n') {
        dlg->result = DIALOG_RES_OK;
        dlg->active = 0;
        play_sound(850); sleep(40); no_sound();
        return 1;
    }

    // Escape: Cancel
    if (scan_code == 0x01) {
        dlg->result = DIALOG_RES_CANCEL;
        dlg->active = 0;
        play_sound(400); sleep(40); no_sound();
        return 1;
    }

    // Tab: Switch field focus
    if (scan_code == 0x0F) {
        if (dlg->field_count > 1) {
            dlg->focused_field = (dlg->focused_field + 1) % dlg->field_count;
            dlg->fields[dlg->focused_field].cursor = dlg->fields[dlg->focused_field].len;
            play_sound(600); sleep(20); no_sound();
        }
        return 1;
    }

    if (dlg->field_count == 0) return 1;
    dialog_field_t* f = &dlg->fields[dlg->focused_field];

    // Backspace
    if (scan_code == 0x0E) {
        if (f->cursor > 0) {
            for (int k = f->cursor - 1; k < f->len; k++) {
                f->value[k] = f->value[k + 1];
            }
            f->cursor--;
            f->len--;
            play_sound(450); sleep(15); no_sound();
        }
        return 1;
    }

    // Delete
    if (scan_code == 0x53) {
        if (f->cursor < f->len) {
            for (int k = f->cursor; k < f->len; k++) {
                f->value[k] = f->value[k + 1];
            }
            f->len--;
            play_sound(450); sleep(15); no_sound();
        }
        return 1;
    }

    // Left arrow
    if (scan_code == 0x4B) {
        if (f->cursor > 0) f->cursor--;
        return 1;
    }

    // Right arrow
    if (scan_code == 0x4D) {
        if (f->cursor < f->len) f->cursor++;
        return 1;
    }

    // Up arrow
    if (scan_code == 0x48) {
        if (dlg->field_count > 1) {
            dlg->focused_field = (dlg->focused_field > 0) ? dlg->focused_field - 1 : dlg->field_count - 1;
            dlg->fields[dlg->focused_field].cursor = dlg->fields[dlg->focused_field].len;
            play_sound(600); sleep(20); no_sound();
        }
        return 1;
    }

    // Down arrow
    if (scan_code == 0x50) {
        if (dlg->field_count > 1) {
            dlg->focused_field = (dlg->focused_field + 1) % dlg->field_count;
            dlg->fields[dlg->focused_field].cursor = dlg->fields[dlg->focused_field].len;
            play_sound(600); sleep(20); no_sound();
        }
        return 1;
    }

    // Home
    if (scan_code == 0x47) {
        f->cursor = 0;
        return 1;
    }

    // End
    if (scan_code == 0x4F) {
        f->cursor = f->len;
        return 1;
    }

    // Printable character
    if (ascii_char >= 32 && ascii_char <= 126) {
        if (f->len < f->max_len && f->len < (DIALOG_FIELD_VALUE_LEN - 1)) {
            for (int k = f->len; k >= f->cursor; k--) {
                f->value[k + 1] = f->value[k];
            }
            f->value[f->cursor] = ascii_char;
            f->cursor++;
            f->len++;
            play_sound(700); sleep(15); no_sound();
        }
        return 1;
    }

    return 1; // Consume key in modal dialog
}

const char* dialog_get_field(dialog_t* dlg, int field_idx) {
    if (!dlg || field_idx < 0 || field_idx >= dlg->field_count) return "";
    return dlg->fields[field_idx].value;
}

void dialog_set_field(dialog_t* dlg, int field_idx, const char* val) {
    if (!dlg || field_idx < 0 || field_idx >= dlg->field_count || !val) return;
    dialog_field_t* f = &dlg->fields[field_idx];
    strncpy(f->value, val, f->max_len);
    f->value[f->max_len] = '\0';
    f->len = (int)strlen(f->value);
    f->cursor = f->len;
}

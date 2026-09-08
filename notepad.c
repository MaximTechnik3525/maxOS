#include "notepad.h"
#include "maxfs.h"

extern unsigned short* _gfx_memory_backend;
#define gfx_memory _gfx_memory_backend

extern int win_x, win_y, win_w, win_h;
extern int pos_x, pos_y;
extern int drag;
extern int cursor_bg_saved;
extern int km_mode;

void print_string(char* str, int x, int y, unsigned short color);
void draw_cursor(int mouse_x, int mouse_y);
void prev_cursor(void);
void draw_window(void);
void play_sound(unsigned int nfreq);
void sleep(unsigned int ms);
void no_sound(void);

int notepad_open = 0;
char ftext[100] = {0};
int textid = 0;
static char np_status[40] = "Ready | Click [Save] to save";
static int current_open_slot = -1;

void notepad_init(void) {
    notepad_open = 0;
    textid = 0;
    current_open_slot = -1;
    for (int i = 0; i < 100; i++) ftext[i] = '\0';
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
    print_string((char*)label, bx + 7, by + 4, text_col);
}

void notepad_set_status(const char* msg) {
    int i = 0;
    while (msg[i] != '\0' && i < 38) {
        np_status[i] = msg[i];
        i++;
    }
    np_status[i] = '\0';
}

static void notepad_render_toolbar(void) {
    int np_x = win_x + 20;
    int np_y = win_y + 38;
    int np_w = win_w - 40;

    // Toolbar background
    draw_rect(np_x + 2, np_y + 22, np_w - 4, 25, 0xDF17);

    // [Save] button (Green fill)
    draw_ui_btn(np_x + 10, np_y + 25, 52, 18, "Save", 0x3DF2, 0x0000);

    // [Clear] button
    draw_ui_btn(np_x + 68, np_y + 25, 56, 18, "Clear", 0xC618, 0x0000);

    // [Open] button
    draw_ui_btn(np_x + 130, np_y + 25, 52, 18, "Open", 0xC618, 0x0000);

    // [Close] button (Soft Red fill)
    draw_ui_btn(np_x + np_w - 65, np_y + 25, 55, 18, "Close", 0xF9A6, 0x0000);

    // Status text
    print_string(np_status, np_x + 192, np_y + 29, 0x0000);
}

static void notepad_draw_toolbar(void) {
    prev_cursor();
    notepad_render_toolbar();
    draw_cursor(pos_x, pos_y);
}

void notepad_draw_text(void) {
    prev_cursor();
    int np_x = win_x + 20;
    int np_y = win_y + 38;
    int np_w = win_w - 40;
    int np_h = win_h - 48;

    // White text area
    draw_rect(np_x + 10, np_y + 50, np_w - 20, np_h - 76, 0xFFFF);
    // Border around text area
    draw_rect(np_x + 10, np_y + 50, np_w - 20, 1, 0x7BEF);
    draw_rect(np_x + 10, np_y + 50, 1, np_h - 76, 0x7BEF);
    draw_rect(np_x + 10, np_y + np_h - 27, np_w - 20, 1, 0x7BEF);
    draw_rect(np_x + np_w - 11, np_y + 50, 1, np_h - 76, 0x7BEF);

    // Format lines (line 1: up to 60 chars, line 2: remaining chars)
    char line1[65];
    char line2[65];
    int l1 = 0, l2 = 0;
    for (int i = 0; i < textid && i < 60; i++) {
        line1[l1++] = ftext[i];
    }
    line1[l1] = '\0';
    for (int i = 60; i < textid && i < 99; i++) {
        line2[l2++] = ftext[i];
    }
    line2[l2] = '\0';

    print_string(line1, np_x + 18, np_y + 60, 0x0000);
    if (textid <= 60) {
        print_string("_", np_x + 18 + textid * 9, np_y + 60, 0x0000);
    } else {
        print_string(line2, np_x + 18, np_y + 78, 0x0000);
        print_string("_", np_x + 18 + (textid - 60) * 9, np_y + 78, 0x0000);
    }

    draw_cursor(pos_x, pos_y);
}

void notepad_draw(void) {
    prev_cursor();
    int np_x = win_x + 20;
    int np_y = win_y + 38;
    int np_w = win_w - 40;
    int np_h = win_h - 48;

    // Window frame
    draw_rect(np_x, np_y, np_w, np_h, 0x0000);
    draw_rect(np_x + 1, np_y + 1, np_w - 2, np_h - 2, 0xEF59);

    // Titlebar
    draw_rect(np_x + 2, np_y + 2, np_w - 4, 19, 0x24EE);
    print_string("maxOS Notepad - Text Editor", np_x + 8, np_y + 6, 0xFFFF);

    // Toolbar
    notepad_render_toolbar();

    // Footer hint bar
    draw_rect(np_x + 2, np_y + np_h - 24, np_w - 4, 22, 0xDF17);
    draw_rect(np_x + 2, np_y + np_h - 24, np_w - 4, 1, 0x9CD3);
    print_string("Type text | Click [Save] to write to maxFS | [Close] or F2 to exit", np_x + 10, np_y + np_h - 18, 0x0000);

    // Text area (internally calls draw_cursor)
    notepad_draw_text();
}

static void notepad_save(void) {
    if (textid == 0) {
        notepad_set_status("Empty! Type something first.");
        play_sound(250); sleep(80); no_sound();
        notepad_draw_toolbar();
        return;
    }

    int slot = current_open_slot;
    if (slot < 0 || slot >= MAXFS_MAX_FILES || !ram_disk[slot].exists) {
        slot = -1;
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            if (!ram_disk[i].exists) {
                slot = i;
                break;
            }
        }
        if (slot == -1) {
            notepad_set_status("maxFS full (5/5)! Disk limit reached.");
            play_sound(200); sleep(150); no_sound();
            notepad_draw_toolbar();
            return;
        }

        char fname[MAXFS_NAME_LEN];
        fname[0] = 'n'; fname[1] = 'o'; fname[2] = 't'; fname[3] = 'e';
        fname[4] = '1' + slot;
        fname[5] = '.'; fname[6] = 't'; fname[7] = 'x'; fname[8] = 't';
        fname[9] = '\0';

        int res = create_file(fname, ftext);
        if (res >= 0) {
            current_open_slot = res;
            np_status[0] = 'S'; np_status[1] = 'a'; np_status[2] = 'v'; np_status[3] = 'e'; np_status[4] = 'd';
            np_status[5] = ':'; np_status[6] = ' ';
            int p = 7;
            for (int i = 0; fname[i] != '\0'; i++) np_status[p++] = fname[i];
            np_status[p++] = ' '; np_status[p++] = '(';
            np_status[p++] = '1' + slot; np_status[p++] = '/'; np_status[p++] = '5';
            np_status[p++] = ')'; np_status[p] = '\0';

            play_sound(700); sleep(60); play_sound(1100); sleep(80); no_sound();
        } else {
            notepad_set_status("Error saving file!");
            play_sound(200); sleep(150); no_sound();
        }
    } else {
        // Update existing file content
        int t = 0;
        while (ftext[t] != '\0' && t < (MAXFS_CONTENT_LEN - 1)) {
            ram_disk[slot].content[t] = ftext[t];
            t++;
        }
        ram_disk[slot].content[t] = '\0';
        ram_disk[slot].size = t;

        np_status[0] = 'U'; np_status[1] = 'p'; np_status[2] = 'd'; np_status[3] = 'a'; np_status[4] = 't'; np_status[5] = 'e'; np_status[6] = 'd';
        np_status[7] = ':'; np_status[8] = ' ';
        int p = 9;
        for (int i = 0; ram_disk[slot].name[i] != '\0'; i++) np_status[p++] = ram_disk[slot].name[i];
        np_status[p] = '\0';

        play_sound(700); sleep(60); play_sound(1100); sleep(80); no_sound();
    }
    notepad_draw_toolbar();
}

static void notepad_clear(void) {
    for (int i = 0; i < 100; i++) ftext[i] = '\0';
    textid = 0;
    current_open_slot = -1;
    notepad_set_status("Editor cleared (new note).");
    play_sound(450); sleep(50); no_sound();
    notepad_draw_toolbar();
    notepad_draw_text();
}

static void notepad_open_file(void) {
    int next_slot = -1;
    for (int step = 1; step <= MAXFS_MAX_FILES; step++) {
        int idx = (current_open_slot + step) % MAXFS_MAX_FILES;
        if (ram_disk[idx].exists) {
            next_slot = idx;
            break;
        }
    }
    if (next_slot == -1) {
        notepad_set_status("No files found on maxFS!");
        play_sound(250); sleep(80); no_sound();
        notepad_draw_toolbar();
        return;
    }
    current_open_slot = next_slot;
    textid = 0;
    while (ram_disk[next_slot].content[textid] != '\0' && textid < 99) {
        ftext[textid] = ram_disk[next_slot].content[textid];
        textid++;
    }
    ftext[textid] = '\0';

    np_status[0] = 'L'; np_status[1] = 'o'; np_status[2] = 'a'; np_status[3] = 'd'; np_status[4] = 'e'; np_status[5] = 'd';
    np_status[6] = ':'; np_status[7] = ' ';
    int p = 8;
    for (int i = 0; ram_disk[next_slot].name[i] != '\0'; i++) np_status[p++] = ram_disk[next_slot].name[i];
    np_status[p] = '\0';

    play_sound(600); sleep(60); no_sound();
    notepad_draw_toolbar();
    notepad_draw_text();
}

void notepad_open_window(void) {
    notepad_open = 1;
    drag = 1;
    current_open_slot = -1;
    notepad_set_status("Ready | Click [Save] to save");
    notepad_draw();
    play_sound(500); sleep(60); no_sound();
}

void notepad_close_window(void) {
    notepad_open = 0;
    drag = 0;
    draw_window();
    play_sound(350); sleep(60); no_sound();
}

int notepad_handle_click(int mouse_x, int mouse_y) {
    if (!notepad_open) return 0;

    int np_x = win_x + 20;
    int np_y = win_y + 38;
    int np_w = win_w - 40;
    int np_h = win_h - 48;

    // Check [Save] button: np_x + 10 .. np_x + 62, np_y + 25 .. np_y + 43
    if (mouse_x >= np_x + 10 && mouse_x <= np_x + 62 && mouse_y >= np_y + 25 && mouse_y <= np_y + 43) {
        notepad_save();
        return 1;
    }
    // Check [Clear] button: np_x + 68 .. np_x + 124, np_y + 25 .. np_y + 43
    if (mouse_x >= np_x + 68 && mouse_x <= np_x + 124 && mouse_y >= np_y + 25 && mouse_y <= np_y + 43) {
        notepad_clear();
        return 1;
    }
    // Check [Open] button: np_x + 130 .. np_x + 182, np_y + 25 .. np_y + 43
    if (mouse_x >= np_x + 130 && mouse_x <= np_x + 182 && mouse_y >= np_y + 25 && mouse_y <= np_y + 43) {
        notepad_open_file();
        return 1;
    }
    // Check [Close] button: np_x + np_w - 65 .. np_x + np_w - 10, np_y + 25 .. np_y + 43
    if (mouse_x >= np_x + np_w - 65 && mouse_x <= np_x + np_w - 10 && mouse_y >= np_y + 25 && mouse_y <= np_y + 43) {
        notepad_close_window();
        return 1;
    }
    // Click inside notepad area (swallow click so it doesn't click underlying elements)
    if (mouse_x >= np_x && mouse_x <= np_x + np_w && mouse_y >= np_y && mouse_y <= np_y + np_h) {
        return 1;
    }
    return 0;
}

int notepad_handle_key(char ascii_char, unsigned char scan_code) {
    if (!notepad_open) return 0;

    // F2 (0x3C) or Escape (0x01) closes Notepad
    if (scan_code == 0x01 || scan_code == 0x3C || ascii_char == 'S') {
        notepad_close_window();
        return 1;
    }

    // Allow keyboard-mouse mode toggles (F3 / 'T' and F4 / 'G')
    if (ascii_char == 'T' || ascii_char == 'G') {
        return 0;
    }

    // Allow keyboard-mouse cursor movement ('U', 'D', 'L', 'R') and click ('e') when km_mode is active
    if (km_mode == 1 && (ascii_char == 'U' || ascii_char == 'D' || ascii_char == 'L' || ascii_char == 'R' || ascii_char == 'e')) {
        return 0;
    }

    // Backspace
    if (scan_code == 0x0E || ascii_char == 'B') {
        if (textid > 0) {
            textid--;
            ftext[textid] = '\0';
            notepad_draw_text();
        }
        return 1;
    }

    // Printable characters
    if ((ascii_char >= 'a' && ascii_char <= 'z') ||
        (ascii_char >= '0' && ascii_char <= '9') ||
        ascii_char == ' ' || ascii_char == '.' || ascii_char == ',' || ascii_char == '/') {
        if (textid < 98) {
            ftext[textid] = ascii_char;
            textid++;
            ftext[textid] = '\0';
            notepad_draw_text();
        }
        return 1;
    }

    // Swallow other keys while notepad is open
    return 1;
}

#include "calc.h"
#include "maxp.h"

extern unsigned short* _gfx_memory_backend;
#define gfx_memory _gfx_memory_backend

extern int win_x, win_y, win_w, win_h;
extern int pos_x, pos_y;
extern int drag;

void print_string(char* str, int x, int y, unsigned short color);
void draw_cursor(int mouse_x, int mouse_y);
void prev_cursor(void);
void draw_window(void);
void play_sound(unsigned int nfreq);
void sleep(unsigned int ms);
void no_sound(void);
void int_str(int num, char* str);

int calc_open = 0;

static long long current_display_val = 0;
static long long stored_val = 0;
static char pending_op = 0; // '+', '-', '*', '/'
static int is_new_entry = 1;
static int is_error = 0;

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

static void draw_calc_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    // 3D button
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
    draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    
    // Centered text
    int len = 0;
    while (label[len] != '\0') len++;
    int tx = bx + (bw - (len * 9)) / 2;
    int ty = by + (bh - 10) / 2;
    print_string((char*)label, tx, ty, text_col);
}

void calc_init(void) {
    calc_open = 0;
    current_display_val = 0;
    stored_val = 0;
    pending_op = 0;
    is_new_entry = 1;
    is_error = 0;
}

void calc_open_window(void) {
    calc_open = 1;
    drag = 1;
    maxp_set_active_app(MAXP_APP_CALC);
    draw_window();
}

void calc_close_window(void) {
    calc_open = 0;
    drag = 0;
    maxp_set_active_app(MAXP_APP_NONE);
    draw_window();
    draw_cursor(pos_x, pos_y);
}

static void calc_execute_pending(void) {
    if (pending_op == 0) {
        stored_val = current_display_val;
        return;
    }
    if (pending_op == '+') {
        stored_val = stored_val + current_display_val;
    } else if (pending_op == '-') {
        stored_val = stored_val - current_display_val;
    } else if (pending_op == '*') {
        stored_val = stored_val * current_display_val;
    } else if (pending_op == '/') {
        if (current_display_val == 0) {
            is_error = 1;
            stored_val = 0;
            current_display_val = 0;
            pending_op = 0;
            is_new_entry = 1;
            return;
        }
        stored_val = stored_val / current_display_val;
    }
    current_display_val = stored_val;
}

static void calc_input_digit(int digit) {
    if (is_error) {
        is_error = 0;
        current_display_val = 0;
    }
    if (is_new_entry) {
        current_display_val = digit;
        is_new_entry = 0;
    } else {
        if (current_display_val < 100000000000LL && current_display_val > -100000000000LL) {
            current_display_val = (current_display_val * 10) + digit;
        }
    }
}

static void calc_input_op(char op) {
    if (is_error) is_error = 0;
    if (!is_new_entry) {
        calc_execute_pending();
    }
    pending_op = op;
    is_new_entry = 1;
}

static void calc_input_equals(void) {
    if (is_error) return;
    calc_execute_pending();
    pending_op = 0;
    is_new_entry = 1;
}

static void calc_clear_all(void) {
    current_display_val = 0;
    stored_val = 0;
    pending_op = 0;
    is_new_entry = 1;
    is_error = 0;
}

void calc_draw(void) {
    prev_cursor();

    int cx = win_x + 190;
    int cy = win_y + 40;
    int cw = 340;
    int ch = 380;

    // Window frame with 3D border
    draw_rect(cx, cy, cw, ch, 0x0000);
    draw_rect(cx + 1, cy + 1, cw - 2, ch - 2, 0xCE79); // Windows 95 / Classic gray
    draw_rect(cx + 1, cy + 1, cw - 3, 1, 0xFFFF);
    draw_rect(cx + 1, cy + 1, 1, ch - 3, 0xFFFF);

    // Titlebar
    draw_rect(cx + 3, cy + 3, cw - 6, 22, 0x11EB); // Navy Blue
    print_string("Calculator - [calc.maxP]", cx + 8, cy + 8, 0xFFFF);

    // [X] Close button
    draw_calc_btn(cx + cw - 24, cy + 4, 18, 18, "X", 0xF9A6, 0x0000);

    // LCD Display Container
    int disp_x = cx + 16;
    int disp_y = cy + 34;
    int disp_w = cw - 32;
    int disp_h = 44;

    // Sunken border
    draw_rect(disp_x, disp_y, disp_w, disp_h, 0x7BEF);
    draw_rect(disp_x, disp_y, disp_w, 1, 0x0000);
    draw_rect(disp_x, disp_y, 1, disp_h, 0x0000);
    draw_rect(disp_x + 1, disp_y + 1, disp_w - 2, disp_h - 2, 0xFFFF);
    draw_rect(disp_x + 2, disp_y + 2, disp_w - 4, disp_h - 4, 0x0124); // Dark emerald LCD

    // Pending operation indicator on left
    char op_str[4] = "  ";
    if (pending_op != 0) {
        op_str[0] = pending_op;
        op_str[1] = '\0';
        print_string(op_str, disp_x + 6, disp_y + 8, 0x07E0); // Bright Green
    }

    // Number text (Right aligned)
    char num_buf[32];
    if (is_error) {
        num_buf[0] = 'E'; num_buf[1] = 'r'; num_buf[2] = 'r'; num_buf[3] = 'o'; num_buf[4] = 'r'; num_buf[5] = '\0';
    } else {
        long long val = current_display_val;
        int is_neg = (val < 0);
        if (is_neg) val = -val;

        char temp[32];
        int tp = 0;
        if (val == 0) {
            temp[tp++] = '0';
        } else {
            while (val > 0) {
                temp[tp++] = (val % 10) + '0';
                val /= 10;
            }
        }
        int np = 0;
        if (is_neg) num_buf[np++] = '-';
        while (tp > 0) {
            num_buf[np++] = temp[--tp];
        }
        num_buf[np] = '\0';
    }

    int text_len = 0;
    while (num_buf[text_len] != '\0') text_len++;
    int num_x = disp_x + disp_w - 12 - (text_len * 9);
    print_string(num_buf, num_x, disp_y + 16, 0xFFFF);

    // Button Grid layout
    int start_bx = cx + 16;
    int start_by = cy + 90;
    int bw = 68;
    int bh = 48;
    int gap_x = 12;
    int gap_y = 8;

    const char* labels[5][4] = {
        { "C",   "CE",  "+/-", "/" },
        { "7",   "8",   "9",   "*" },
        { "4",   "5",   "6",   "-" },
        { "1",   "2",   "3",   "+" },
        { "0",   "00",  "BS",  "=" }
    };

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = start_bx + c * (bw + gap_x);
            int by = start_by + r * (bh + gap_y);
            const char* lbl = labels[r][c];

            unsigned short fill = 0xE71C; // standard button
            unsigned short text_col = 0x0000;

            if (lbl[0] == 'C' && lbl[1] == '\0') {
                fill = 0xF9A6; // Red Clear
            } else if (lbl[0] == '=') {
                fill = 0x24EE; // Cyan Equals
                text_col = 0x0000;
            } else if (lbl[0] == '/' || lbl[0] == '*' || lbl[0] == '-' || lbl[0] == '+') {
                fill = 0xFCEF; // Operator button
            }

            draw_calc_btn(bx, by, bw, bh, lbl, fill, text_col);
        }
    }

    draw_cursor(pos_x, pos_y);
}

int calc_handle_click(int mouse_x, int mouse_y) {
    if (!calc_open) return 0;

    int cx = win_x + 190;
    int cy = win_y + 40;
    int cw = 340;
    int ch = 380;

    // [X] Close button
    if (mouse_x >= cx + cw - 24 && mouse_x <= cx + cw - 6 && mouse_y >= cy + 4 && mouse_y <= cy + 22) {
        calc_close_window();
        return 1;
    }

    // Buttons
    int start_bx = cx + 16;
    int start_by = cy + 90;
    int bw = 68;
    int bh = 48;
    int gap_x = 12;
    int gap_y = 8;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = start_bx + c * (bw + gap_x);
            int by = start_by + r * (bh + gap_y);

            if (mouse_x >= bx && mouse_x <= bx + bw && mouse_y >= by && mouse_y <= by + bh) {
                play_sound(800); sleep(20); no_sound();
                if (r == 0) {
                    if (c == 0) calc_clear_all();
                    else if (c == 1) { current_display_val = 0; is_new_entry = 1; }
                    else if (c == 2) { current_display_val = -current_display_val; }
                    else if (c == 3) calc_input_op('/');
                } else if (r == 1) {
                    if (c == 0) calc_input_digit(7);
                    else if (c == 1) calc_input_digit(8);
                    else if (c == 2) calc_input_digit(9);
                    else if (c == 3) calc_input_op('*');
                } else if (r == 2) {
                    if (c == 0) calc_input_digit(4);
                    else if (c == 1) calc_input_digit(5);
                    else if (c == 2) calc_input_digit(6);
                    else if (c == 3) calc_input_op('-');
                } else if (r == 3) {
                    if (c == 0) calc_input_digit(1);
                    else if (c == 1) calc_input_digit(2);
                    else if (c == 2) calc_input_digit(3);
                    else if (c == 3) calc_input_op('+');
                } else if (r == 4) {
                    if (c == 0) calc_input_digit(0);
                    else if (c == 1) { calc_input_digit(0); calc_input_digit(0); }
                    else if (c == 2) { current_display_val /= 10; }
                    else if (c == 3) calc_input_equals();
                }
                calc_draw();
                return 1;
            }
        }
    }

    // Swallow any click within calculator window
    if (mouse_x >= cx && mouse_x <= cx + cw && mouse_y >= cy && mouse_y <= cy + ch) {
        return 1;
    }

    return 0;
}

int calc_handle_key(char ascii_char, unsigned char scan_code) {
    if (!calc_open) return 0;

    // Digits
    if (ascii_char >= '0' && ascii_char <= '9') {
        calc_input_digit(ascii_char - '0');
        calc_draw();
        play_sound(800); sleep(15); no_sound();
        return 1;
    }

    // Operators
    if (ascii_char == '+' || ascii_char == '-' || ascii_char == '*' || ascii_char == '/') {
        calc_input_op(ascii_char);
        calc_draw();
        play_sound(900); sleep(15); no_sound();
        return 1;
    }

    // Equals / Enter
    if (ascii_char == '=' || scan_code == 0x1C) { // Enter
        calc_input_equals();
        calc_draw();
        play_sound(1000); sleep(20); no_sound();
        return 1;
    }

    // Clear
    if (ascii_char == 'c' || ascii_char == 'C') {
        calc_clear_all();
        calc_draw();
        return 1;
    }

    // Backspace
    if (scan_code == 0x0E) {
        current_display_val /= 10;
        calc_draw();
        return 1;
    }

    // Close window: Esc (0x01) or 'q'
    if (scan_code == 0x01 || ascii_char == 'q' || ascii_char == 'Q') {
        calc_close_window();
        return 1;
    }

    return 0;
}

#include "calc.h"
#include "maxp.h"
#include "kernel.h"

int calc_open = 0;
static calc_state_t primary_calc_state;

/* -------------------------------------------------------------------------
 * Internal String & Floating Point Helpers (freestanding x86_64)
 * ------------------------------------------------------------------------- */
static int calc_strlen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

static void calc_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static double str_to_double(const char* s) {
    double res = 0.0;
    double frac = 0.0;
    double div = 1.0;
    int is_neg = 0;
    int in_frac = 0;

    if (*s == '-') {
        is_neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s) {
        if (*s == '.') {
            in_frac = 1;
        } else if (*s >= '0' && *s <= '9') {
            if (!in_frac) {
                res = res * 10.0 + (*s - '0');
            } else {
                div *= 10.0;
                frac += (*s - '0') / div;
            }
        }
        s++;
    }

    res += frac;
    return is_neg ? -res : res;
}

static void double_to_str(double val, char* buf, int max_decimals) {
    if (val != val) {
        calc_strcpy(buf, "Error");
        return;
    }
    if (val > 1e12 || val < -1e12) {
        calc_strcpy(buf, "Overflow");
        return;
    }

    int is_neg = 0;
    if (val < 0.0) {
        is_neg = 1;
        val = -val;
    }

    double rounder = 0.5;
    for (int i = 0; i < max_decimals; i++) {
        rounder /= 10.0;
    }
    val += rounder;

    long long int_part = (long long)val;
    double frac_part = val - (double)int_part;

    char temp[32];
    int tp = 0;
    if (int_part == 0) {
        temp[tp++] = '0';
    } else {
        long long v = int_part;
        while (v > 0) {
            temp[tp++] = (char)((v % 10) + '0');
            v /= 10;
        }
    }

    char frac_buf[16];
    int fp = 0;
    for (int i = 0; i < max_decimals; i++) {
        frac_part *= 10.0;
        int digit = (int)frac_part;
        if (digit > 9) digit = 9;
        frac_buf[fp++] = (char)(digit + '0');
        frac_part -= (double)digit;
    }

    while (fp > 0 && frac_buf[fp - 1] == '0') {
        fp--;
    }

    int bp = 0;
    if (is_neg && !(int_part == 0 && fp == 0)) {
        buf[bp++] = '-';
    }
    if (int_part == 0) {
        buf[bp++] = '0';
    } else {
        while (tp > 0) {
            buf[bp++] = temp[--tp];
        }
    }
    if (fp > 0) {
        buf[bp++] = '.';
        for (int i = 0; i < fp; i++) {
            buf[bp++] = frac_buf[i];
        }
    }
    buf[bp] = '\0';
}

/* -------------------------------------------------------------------------
 * Instance Operations
 * ------------------------------------------------------------------------- */
void calc_instance_init(calc_state_t* s) {
    s->stored_val = 0.0;
    s->pending_op = 0;
    s->is_error = 0;
    s->is_new_entry = 1;
    s->entry_buf[0] = '0';
    s->entry_buf[1] = '\0';
}

static void calc_clear_all(calc_state_t* s) {
    s->stored_val = 0.0;
    s->pending_op = 0;
    s->is_error = 0;
    s->is_new_entry = 1;
    s->entry_buf[0] = '0';
    s->entry_buf[1] = '\0';
}

static void calc_clear_entry(calc_state_t* s) {
    s->is_error = 0;
    s->is_new_entry = 1;
    s->entry_buf[0] = '0';
    s->entry_buf[1] = '\0';
}

static void calc_backspace(calc_state_t* s) {
    if (s->is_error) {
        calc_clear_all(s);
        return;
    }
    if (s->is_new_entry) return;

    int len = calc_strlen(s->entry_buf);
    if (len > 0) {
        s->entry_buf[len - 1] = '\0';
    }
    if (s->entry_buf[0] == '\0' || (s->entry_buf[0] == '-' && s->entry_buf[1] == '\0')) {
        s->entry_buf[0] = '0';
        s->entry_buf[1] = '\0';
        s->is_new_entry = 1;
    }
}

static void calc_negate(calc_state_t* s) {
    if (s->is_error) return;
    if (s->entry_buf[0] == '0' && s->entry_buf[1] == '\0') return;

    if (s->entry_buf[0] == '-') {
        int len = calc_strlen(s->entry_buf);
        for (int i = 0; i < len; i++) {
            s->entry_buf[i] = s->entry_buf[i + 1];
        }
    } else {
        int len = calc_strlen(s->entry_buf);
        if (len < 15) {
            for (int i = len; i >= 0; i--) {
                s->entry_buf[i + 1] = s->entry_buf[i];
            }
            s->entry_buf[0] = '-';
        }
    }
}

static void calc_input_digit(calc_state_t* s, int digit) {
    if (s->is_error) {
        calc_clear_all(s);
    }
    if (s->is_new_entry) {
        s->entry_buf[0] = (char)('0' + digit);
        s->entry_buf[1] = '\0';
        s->is_new_entry = 0;
    } else {
        int len = calc_strlen(s->entry_buf);
        if (len < 14) {
            if (len == 1 && s->entry_buf[0] == '0') {
                s->entry_buf[0] = (char)('0' + digit);
            } else {
                s->entry_buf[len] = (char)('0' + digit);
                s->entry_buf[len + 1] = '\0';
            }
        }
    }
}

static void calc_input_dot(calc_state_t* s) {
    if (s->is_error) {
        calc_clear_all(s);
    }
    if (s->is_new_entry) {
        s->entry_buf[0] = '0';
        s->entry_buf[1] = '.';
        s->entry_buf[2] = '\0';
        s->is_new_entry = 0;
        return;
    }
    for (int i = 0; s->entry_buf[i] != '\0'; i++) {
        if (s->entry_buf[i] == '.') return;
    }
    int len = calc_strlen(s->entry_buf);
    if (len < 14) {
        s->entry_buf[len] = '.';
        s->entry_buf[len + 1] = '\0';
    }
}

static void calc_execute_pending(calc_state_t* s) {
    if (s->pending_op == 0) return;

    double cur = str_to_double(s->entry_buf);

    if (s->pending_op == '+') {
        s->stored_val += cur;
    } else if (s->pending_op == '-') {
        s->stored_val -= cur;
    } else if (s->pending_op == '*') {
        s->stored_val *= cur;
    } else if (s->pending_op == '/') {
        if (cur == 0.0) {
            calc_strcpy(s->entry_buf, "Cannot divide by 0");
            s->is_error = 1;
            s->is_new_entry = 1;
            s->pending_op = 0;
            return;
        }
        s->stored_val /= cur;
    }

    double_to_str(s->stored_val, s->entry_buf, 6);
}

static void calc_input_op(calc_state_t* s, char op) {
    if (s->is_error) s->is_error = 0;
    if (!s->is_new_entry) {
        calc_execute_pending(s);
    } else if (s->pending_op == 0) {
        s->stored_val = str_to_double(s->entry_buf);
    }
    s->pending_op = op;
    s->is_new_entry = 1;
}

static void calc_input_equals(calc_state_t* s) {
    if (s->is_error) return;
    calc_execute_pending(s);
    s->pending_op = 0;
    s->is_new_entry = 1;
}

/* -------------------------------------------------------------------------
 * UI Drawing
 * ------------------------------------------------------------------------- */
static void draw_calc_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
    draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);

    int len = calc_strlen(label);
    int tx = bx + (bw - (len * 9)) / 2;
    int ty = by + (bh - 10) / 2;
    print_string((char*)label, tx, ty, text_col);
}

void calc_instance_draw(calc_state_t* s, int cx, int cy, int cw, int ch) {
    prev_cursor();

    // Window frame with classic 3D border
    draw_rect(cx, cy, cw, ch, 0x0000);
    draw_rect(cx + 1, cy + 1, cw - 2, ch - 2, 0xCE79);
    draw_rect(cx + 1, cy + 1, cw - 3, 1, 0xFFFF);
    draw_rect(cx + 1, cy + 1, 1, ch - 3, 0xFFFF);

    // Titlebar
    draw_rect(cx + 3, cy + 3, cw - 6, 22, 0x11EB);
    print_string("Calculator 3.1 - [calc.bin]", cx + 8, cy + 8, 0xFFFF);

    // [X] Close button
    draw_calc_btn(cx + cw - 24, cy + 4, 18, 18, "X", 0xF800, 0xFFFF);

    // LCD Display Container
    int disp_x = cx + 16;
    int disp_y = cy + 34;
    int disp_w = cw - 32;
    int disp_h = 44;

    draw_rect(disp_x, disp_y, disp_w, disp_h, 0x7BEF);
    draw_rect(disp_x, disp_y, disp_w, 1, 0x0000);
    draw_rect(disp_x, disp_y, 1, disp_h, 0x0000);
    draw_rect(disp_x + 1, disp_y + 1, disp_w - 2, disp_h - 2, 0xFFFF);
    draw_rect(disp_x + 2, disp_y + 2, disp_w - 4, disp_h - 4, 0x0124);

    if (s->pending_op != 0) {
        char op_str[4];
        op_str[0] = s->pending_op;
        op_str[1] = '\0';
        print_string(op_str, disp_x + 8, disp_y + 16, 0x07E0);
    }

    int txt_len = calc_strlen(s->entry_buf);
    int txt_x = disp_x + disp_w - (txt_len * 9) - 10;
    if (txt_x < disp_x + 26) txt_x = disp_x + 26;
    print_string(s->entry_buf, txt_x, disp_y + 16, 0xFFFF);

    // Button Grid: 5 rows x 4 columns
    const char* labels[5][4] = {
        { "C",   "CE",  "<-",  "/" },
        { "7",   "8",   "9",   "*" },
        { "4",   "5",   "6",   "-" },
        { "1",   "2",   "3",   "+" },
        { "+/-", "0",   ".",   "=" }
    };

    unsigned short fills[5][4] = {
        { 0xCE79, 0xCE79, 0xCE79, 0xF621 },
        { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
        { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
        { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
        { 0xCE79, 0xFFFF, 0xCE79, 0x03EA }
    };

    int start_gx = cx + 16;
    int start_gy = cy + 90;
    int btn_w = 68;
    int btn_h = 50;
    int gap_x = 12;
    int gap_y = 6;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = start_gx + c * (btn_w + gap_x);
            int by = start_gy + r * (btn_h + gap_y);
            unsigned short text_col = (r == 4 && c == 3) ? 0xFFFF : 0x0000;
            draw_calc_btn(bx, by, btn_w, btn_h, labels[r][c], fills[r][c], text_col);
        }
    }

    draw_cursor(pos_x, pos_y);
}

int calc_instance_click(calc_state_t* s, int cx, int cy, int cw, int ch, int mouse_x, int mouse_y) {
    // [X] Titlebar Close Button
    if (mouse_x >= cx + cw - 26 && mouse_x <= cx + cw && mouse_y >= cy && mouse_y <= cy + 24) {
        return -1; // Request close
    }

    int start_gx = cx + 16;
    int start_gy = cy + 90;
    int btn_w = 68;
    int btn_h = 50;
    int gap_x = 12;
    int gap_y = 6;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = start_gx + c * (btn_w + gap_x);
            int by = start_gy + r * (btn_h + gap_y);

            if (mouse_x >= bx && mouse_x <= bx + btn_w && mouse_y >= by && mouse_y <= by + btn_h) {
                play_sound(750); sleep(20); no_sound();

                if (r == 0) {
                    if (c == 0) calc_clear_all(s);
                    else if (c == 1) calc_clear_entry(s);
                    else if (c == 2) calc_backspace(s);
                    else if (c == 3) calc_input_op(s, '/');
                } else if (r == 1) {
                    if (c == 0) calc_input_digit(s, 7);
                    else if (c == 1) calc_input_digit(s, 8);
                    else if (c == 2) calc_input_digit(s, 9);
                    else if (c == 3) calc_input_op(s, '*');
                } else if (r == 2) {
                    if (c == 0) calc_input_digit(s, 4);
                    else if (c == 1) calc_input_digit(s, 5);
                    else if (c == 2) calc_input_digit(s, 6);
                    else if (c == 3) calc_input_op(s, '-');
                } else if (r == 3) {
                    if (c == 0) calc_input_digit(s, 1);
                    else if (c == 1) calc_input_digit(s, 2);
                    else if (c == 2) calc_input_digit(s, 3);
                    else if (c == 3) calc_input_op(s, '+');
                } else if (r == 4) {
                    if (c == 0) calc_negate(s);
                    else if (c == 1) calc_input_digit(s, 0);
                    else if (c == 2) calc_input_dot(s);
                    else if (c == 3) calc_input_equals(s);
                }
                draw_window();
                return 1;
            }
        }
    }

    if (mouse_x >= cx && mouse_x <= cx + cw && mouse_y >= cy && mouse_y <= cy + ch) {
        return 1;
    }

    return 0;
}

int calc_instance_key(calc_state_t* s, char ascii_char, unsigned char scan_code) {
    if (ascii_char >= '0' && ascii_char <= '9') {
        calc_input_digit(s, ascii_char - '0');
        draw_window();
        play_sound(800); sleep(15); no_sound();
        return 1;
    }

    if (ascii_char == '.' || ascii_char == ',' || scan_code == 0x34 || scan_code == 0x33) {
        calc_input_dot(s);
        draw_window();
        play_sound(800); sleep(15); no_sound();
        return 1;
    }

    if (ascii_char == '+' || ascii_char == '-' || ascii_char == '*' || ascii_char == '/') {
        calc_input_op(s, ascii_char);
        draw_window();
        play_sound(900); sleep(15); no_sound();
        return 1;
    }

    if (ascii_char == '=' || scan_code == 0x1C || ascii_char == '\n') {
        calc_input_equals(s);
        draw_window();
        play_sound(1000); sleep(20); no_sound();
        return 1;
    }

    if (ascii_char == 'c' || ascii_char == 'C' || scan_code == 0x53) {
        calc_clear_all(s);
        draw_window();
        return 1;
    }

    if (ascii_char == 'e' || ascii_char == 'E') {
        calc_clear_entry(s);
        draw_window();
        return 1;
    }

    if (scan_code == 0x0E || ascii_char == 'B') {
        calc_backspace(s);
        draw_window();
        return 1;
    }

    if (ascii_char == 'n' || ascii_char == 'N' || ascii_char == '_') {
        calc_negate(s);
        draw_window();
        return 1;
    }

    if (scan_code == 0x01 || ascii_char == 'q' || ascii_char == 'Q') {
        return -1; // Request close
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void calc_init(void) {
    calc_open = 0;
    calc_instance_init(&primary_calc_state);
}

void calc_open_window(void) {
    calc_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_CALC, "Calculator", 0);
}

void calc_close_window(void) {
    calc_open = 0;
    maxp_close_app(MAXP_APP_CALC);
}

void calc_draw(void) {
    calc_instance_draw(&primary_calc_state, win_x + 190, win_y + 40, 340, 380);
}

int calc_handle_click(int mouse_x, int mouse_y) {
    int res = calc_instance_click(&primary_calc_state, win_x + 190, win_y + 40, 340, 380, mouse_x, mouse_y);
    if (res == -1) {
        calc_close_window();
        return 1;
    }
    return res;
}

int calc_handle_key(char ascii_char, unsigned char scan_code) {
    int res = calc_instance_key(&primary_calc_state, ascii_char, scan_code);
    if (res == -1) {
        calc_close_window();
        return 1;
    }
    return res;
}

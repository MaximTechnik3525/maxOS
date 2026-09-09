#include "calc.h"
#include "maxp.h"
#include "kernel.h"

int calc_open = 0;

static char entry_buf[32] = "0";
static double stored_val = 0.0;
static char pending_op = 0; // '+', '-', '*', '/'
static int is_new_entry = 1;
static int is_error = 0;

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
    // Check for NaN or Overflow
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

    // Apply half-up rounding for max_decimals
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

    // Strip trailing zeros from fractional part
    while (fp > 0 && frac_buf[fp - 1] == '0') {
        fp--;
    }

    int bp = 0;
    if (is_neg && !(int_part == 0 && fp == 0)) {
        buf[bp++] = '-';
    }
    while (tp > 0) {
        buf[bp++] = temp[--tp];
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
 * Calculator Engine Functions
 * ------------------------------------------------------------------------- */
static void calc_clear_all(void) {
    stored_val = 0.0;
    pending_op = 0;
    is_error = 0;
    is_new_entry = 1;
    entry_buf[0] = '0';
    entry_buf[1] = '\0';
}

static void calc_clear_entry(void) {
    is_error = 0;
    is_new_entry = 1;
    entry_buf[0] = '0';
    entry_buf[1] = '\0';
}

static void calc_backspace(void) {
    if (is_error) {
        calc_clear_all();
        return;
    }
    if (is_new_entry) return;

    int len = calc_strlen(entry_buf);
    if (len > 0) {
        entry_buf[len - 1] = '\0';
    }
    if (entry_buf[0] == '\0' || (entry_buf[0] == '-' && entry_buf[1] == '\0')) {
        entry_buf[0] = '0';
        entry_buf[1] = '\0';
        is_new_entry = 1;
    }
}

static void calc_negate(void) {
    if (is_error) return;
    if (entry_buf[0] == '0' && entry_buf[1] == '\0') return;

    if (entry_buf[0] == '-') {
        int len = calc_strlen(entry_buf);
        for (int i = 0; i < len; i++) {
            entry_buf[i] = entry_buf[i + 1];
        }
    } else {
        int len = calc_strlen(entry_buf);
        if (len < 15) {
            for (int i = len; i >= 0; i--) {
                entry_buf[i + 1] = entry_buf[i];
            }
            entry_buf[0] = '-';
        }
    }
}

static void calc_input_digit(int digit) {
    if (is_error) {
        calc_clear_all();
    }
    if (is_new_entry) {
        entry_buf[0] = (char)('0' + digit);
        entry_buf[1] = '\0';
        is_new_entry = 0;
    } else {
        int len = calc_strlen(entry_buf);
        if (len < 14) {
            if (len == 1 && entry_buf[0] == '0') {
                entry_buf[0] = (char)('0' + digit);
            } else {
                entry_buf[len] = (char)('0' + digit);
                entry_buf[len + 1] = '\0';
            }
        }
    }
}

static void calc_input_dot(void) {
    if (is_error) {
        calc_clear_all();
    }
    if (is_new_entry) {
        entry_buf[0] = '0';
        entry_buf[1] = '.';
        entry_buf[2] = '\0';
        is_new_entry = 0;
        return;
    }

    // Check if dot already exists
    int has_dot = 0;
    for (int i = 0; entry_buf[i] != '\0'; i++) {
        if (entry_buf[i] == '.') {
            has_dot = 1;
            break;
        }
    }
    if (!has_dot) {
        int len = calc_strlen(entry_buf);
        if (len < 13) {
            entry_buf[len] = '.';
            entry_buf[len + 1] = '\0';
        }
    }
}

static void calc_execute_pending(void) {
    double cur = str_to_double(entry_buf);
    if (pending_op == 0) {
        stored_val = cur;
        return;
    }

    if (pending_op == '+') {
        stored_val += cur;
    } else if (pending_op == '-') {
        stored_val -= cur;
    } else if (pending_op == '*') {
        stored_val *= cur;
    } else if (pending_op == '/') {
        if (cur == 0.0 || cur == -0.0) {
            is_error = 1;
            calc_strcpy(entry_buf, "Error");
            pending_op = 0;
            is_new_entry = 1;
            return;
        }
        stored_val /= cur;
    }

    double_to_str(stored_val, entry_buf, 6);
}

static void calc_input_op(char op) {
    if (is_error) is_error = 0;
    if (!is_new_entry) {
        calc_execute_pending();
    } else if (pending_op == 0) {
        stored_val = str_to_double(entry_buf);
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

/* -------------------------------------------------------------------------
 * UI Drawing
 * ------------------------------------------------------------------------- */
static void draw_calc_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    // 3D beveled button
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + bw - 2, by + 1, 1, bh - 2, 0x7BEF);
    draw_rect(bx + 1, by + bh - 2, bw - 2, 1, 0x7BEF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);

    // Centered label
    int len = calc_strlen(label);
    int tx = bx + (bw - (len * 9)) / 2;
    int ty = by + (bh - 10) / 2;
    print_string((char*)label, tx, ty, text_col);
}

void calc_init(void) {
    calc_open = 0;
    calc_clear_all();
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

void calc_draw(void) {
    prev_cursor();

    int cx = win_x + 190;
    int cy = win_y + 40;
    int cw = 340;
    int ch = 380;

    // Window frame with classic 3D border
    draw_rect(cx, cy, cw, ch, 0x0000);
    draw_rect(cx + 1, cy + 1, cw - 2, ch - 2, 0xCE79); // Windows gray
    draw_rect(cx + 1, cy + 1, cw - 3, 1, 0xFFFF);
    draw_rect(cx + 1, cy + 1, 1, ch - 3, 0xFFFF);

    // Titlebar
    draw_rect(cx + 3, cy + 3, cw - 6, 22, 0x11EB); // Navy Blue
    print_string("Calculator 3.0 - [calc.maxP]", cx + 8, cy + 8, 0xFFFF);

    // [X] Close button
    draw_calc_btn(cx + cw - 24, cy + 4, 18, 18, "X", 0xF9A6, 0x0000);

    // LCD Display Container
    int disp_x = cx + 16;
    int disp_y = cy + 34;
    int disp_w = cw - 32;
    int disp_h = 44;

    // Sunken border & dark emerald screen
    draw_rect(disp_x, disp_y, disp_w, disp_h, 0x7BEF);
    draw_rect(disp_x, disp_y, disp_w, 1, 0x0000);
    draw_rect(disp_x, disp_y, 1, disp_h, 0x0000);
    draw_rect(disp_x + 1, disp_y + 1, disp_w - 2, disp_h - 2, 0xFFFF);
    draw_rect(disp_x + 2, disp_y + 2, disp_w - 4, disp_h - 4, 0x0124); // Dark emerald LCD

    // Pending operation indicator on the left
    if (pending_op != 0) {
        char op_str[4];
        op_str[0] = pending_op;
        op_str[1] = '\0';
        print_string(op_str, disp_x + 8, disp_y + 16, 0x07E0); // Bright Green LCD
    }

    // Number text (Right aligned)
    int text_len = calc_strlen(entry_buf);
    int num_x = disp_x + disp_w - 12 - (text_len * 9);
    unsigned short num_color = is_error ? 0xF800 : 0xFFFF; // Red on error, White normally
    print_string(entry_buf, num_x, disp_y + 16, num_color);

    // 5x4 Button Grid layout
    int start_bx = cx + 16;
    int start_by = cy + 90;
    int bw = 68;
    int bh = 48;
    int gap_x = 12;
    int gap_y = 8;

    const char* labels[5][4] = {
        { "C",   "CE",  "BS",  "/" },
        { "7",   "8",   "9",   "*" },
        { "4",   "5",   "6",   "-" },
        { "1",   "2",   "3",   "+" },
        { "+/-", "0",   ".",   "=" }
    };

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = start_bx + c * (bw + gap_x);
            int by = start_by + r * (bh + gap_y);
            const char* lbl = labels[r][c];

            unsigned short fill = 0xF7BE; // Clean light gray button
            unsigned short text_col = 0x0000;

            if (lbl[0] == 'C' && lbl[1] == '\0') {
                fill = 0xF9A6; // Soft Red (Clear)
            } else if (lbl[0] == 'C' && lbl[1] == 'E') {
                fill = 0xFDC8; // Soft Orange (Clear Entry)
            } else if (lbl[0] == 'B' && lbl[1] == 'S') {
                fill = 0xDF17; // Gray (Backspace)
            } else if (lbl[0] == '=') {
                fill = 0x24EE; // Vibrant Cyan (Equals)
            } else if (lbl[0] == '/' || lbl[0] == '*' || lbl[0] == '-' || lbl[0] == '+') {
                fill = 0xCE79; // Slate Operator button
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

    // Button Grid
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
                    else if (c == 1) calc_clear_entry();
                    else if (c == 2) calc_backspace();
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
                    if (c == 0) calc_negate();
                    else if (c == 1) calc_input_digit(0);
                    else if (c == 2) calc_input_dot();
                    else if (c == 3) calc_input_equals();
                }
                calc_draw();
                return 1;
            }
        }
    }

    // Consume clicks within window bounds
    if (mouse_x >= cx && mouse_x <= cx + cw && mouse_y >= cy && mouse_y <= cy + ch) {
        return 1;
    }

    return 0;
}

int calc_handle_key(char ascii_char, unsigned char scan_code) {
    if (!calc_open) return 0;

    // Digits 0..9
    if (ascii_char >= '0' && ascii_char <= '9') {
        calc_input_digit(ascii_char - '0');
        calc_draw();
        play_sound(800); sleep(15); no_sound();
        return 1;
    }

    // Decimal point (dot or comma)
    if (ascii_char == '.' || ascii_char == ',' || scan_code == 0x34 || scan_code == 0x33) {
        calc_input_dot();
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
    if (ascii_char == '=' || scan_code == 0x1C || ascii_char == '\n') {
        calc_input_equals();
        calc_draw();
        play_sound(1000); sleep(20); no_sound();
        return 1;
    }

    // Clear all ('c', 'C', or Delete key scan_code 0x53)
    if (ascii_char == 'c' || ascii_char == 'C' || scan_code == 0x53) {
        calc_clear_all();
        calc_draw();
        return 1;
    }

    // Clear entry ('e', 'E')
    if (ascii_char == 'e' || ascii_char == 'E') {
        calc_clear_entry();
        calc_draw();
        return 1;
    }

    // Backspace
    if (scan_code == 0x0E || ascii_char == 'B') {
        calc_backspace();
        calc_draw();
        return 1;
    }

    // Negate ('n', 'N', or '_')
    if (ascii_char == 'n' || ascii_char == 'N' || ascii_char == '_') {
        calc_negate();
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

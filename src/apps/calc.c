#include "calc.h"
#include "../user/libc/include/maxos.h"
#include "../user/libc/include/string.h"
#include "../user/libc/include/stdlib.h"
#include "maxp.h"

// We are migrating this to an independent Event Loop application!
calc_state_t primary_calc_state;
int calc_open = 0;

static const char* calc_labels[5][4] = {
    { "C",   "CE",  "BSP", "/" },
    { "7",   "8",   "9",   "*" },
    { "4",   "5",   "6",   "-" },
    { "1",   "2",   "3",   "+" },
    { "+/-", "0",   ".",   "=" }
};

static const unsigned short calc_fills[5][4] = {
    { 0xCE79, 0xCE79, 0xCE79, 0xF621 },
    { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
    { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
    { 0xFFFF, 0xFFFF, 0xFFFF, 0xF621 },
    { 0xCE79, 0xFFFF, 0xCE79, 0x03EA }
};

static void f_to_str(double val, char* buf) {
    // Simple float to string converter (integer part only for now, since we don't have full sprintf)
    // Actually, maxOS apps have limited libc. For now we just implement simple logic.
    if (val < 0) {
        *buf++ = '-';
        val = -val;
    }
    long int_part = (long)val;
    long temp = int_part;
    int len = 0;
    if (temp == 0) len = 1;
    while(temp > 0) { len++; temp /= 10; }
    
    buf[len] = '\0';
    temp = int_part;
    if (temp == 0) {
        buf[0] = '0';
    } else {
        for(int i = len - 1; i >= 0; i--) {
            buf[i] = '0' + (temp % 10);
            temp /= 10;
        }
    }
}

static double str_to_f(const char* buf) {
    double val = 0;
    int sign = 1;
    if (*buf == '-') { sign = -1; buf++; }
    while (*buf >= '0' && *buf <= '9') {
        val = val * 10 + (*buf - '0');
        buf++;
    }
    // Fraction logic can be added later
    return val * sign;
}

void calc_instance_init(calc_state_t* s) {
    s->entry_buf[0] = '0';
    s->entry_buf[1] = '\0';
    s->stored_val = 0;
    s->pending_op = 0;
    s->is_new_entry = 1;
    s->is_error = 0;
}

static void calc_input_digit(calc_state_t* s, int d) {
    if (s->is_error) return;
    if (s->is_new_entry) {
        s->entry_buf[0] = '0' + d;
        s->entry_buf[1] = '\0';
        s->is_new_entry = 0;
    } else {
        int len = strlen(s->entry_buf);
        if (len < 15) {
            s->entry_buf[len] = '0' + d;
            s->entry_buf[len+1] = '\0';
        }
    }
}

static void calc_input_op(calc_state_t* s, char op) {
    if (s->is_error) return;
    s->stored_val = str_to_f(s->entry_buf);
    s->pending_op = op;
    s->is_new_entry = 1;
}

static void calc_input_equals(calc_state_t* s) {
    if (s->is_error || !s->pending_op) return;
    double current = str_to_f(s->entry_buf);
    double res = 0;
    if (s->pending_op == '+') res = s->stored_val + current;
    if (s->pending_op == '-') res = s->stored_val - current;
    if (s->pending_op == '*') res = s->stored_val * current;
    if (s->pending_op == '/') {
        if (current == 0) { s->is_error = 1; strcpy(s->entry_buf, "Error"); return; }
        res = s->stored_val / current;
    }
    f_to_str(res, s->entry_buf);
    s->pending_op = 0;
    s->is_new_entry = 1;
}

static void calc_clear_all(calc_state_t* s) {
    calc_instance_init(s);
}

static void draw_ui_btn(int x, int y, int w, int h, const char* label, unsigned short fill, unsigned short text_col) {
    maxos_draw_rect(x, y, w, h, 0x0000);
    maxos_draw_rect(x + 1, y + 1, w - 2, h - 2, fill);
    int len = strlen(label);
    int tx = x + (w - len * 9) / 2;
    int ty = y + (h - 8) / 2;
    maxos_print_text(label, tx, ty, text_col);
}

static void calc_render_lcd(int cx, int cy, int cw, const char* buf) {
    maxos_draw_rect(cx + 16, cy + 32, cw - 32, 48, 0x0000);
    maxos_print_text(buf, cx + 26, cy + 52, 0xFFFF);
}

static void calc_render_window(int cx, int cy, int cw, int ch, calc_state_t* s) {
    maxos_draw_rect(cx, cy, cw, ch, 0x0000);
    maxos_draw_rect(cx + 1, cy + 1, cw - 2, ch - 2, 0xCE79);
    maxos_draw_rect(cx + 3, cy + 3, cw - 6, 22, 0x11EB);
    maxos_print_text("Calculator 3.5 - [Event Loop]", cx + 8, cy + 10, 0xFFFF);

    draw_ui_btn(cx + cw - 44, cy + 6, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(cx + cw - 22, cy + 6, 18, 16, "X", 0xF800, 0xFFFF);
    
    // LCD
    calc_render_lcd(cx, cy, cw, s->entry_buf);

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = cx + 16 + c * 80;
            int by = cy + 90 + r * 56;
            draw_ui_btn(bx, by, 68, 50, calc_labels[r][c], calc_fills[r][c], (r==4&&c==3)?0xFFFF:0x0000);
        }
    }
}

void calc_main(void) {
    calc_state_t state;
    calc_instance_init(&state);
    
    int cx = 180, cy = 60, cw = 340, ch = 380;
    maxos_debug_log("CALC", "Calculator app started in Ring 3 Event Loop");

    while (1) {
        maxos_event_t ev;
        if (maxos_get_event(&ev)) {
            if (ev.type == EVENT_DRAW) {
                if (ev.x != 0 || ev.y != 0) {
                    cx = ev.x; cy = ev.y; cw = ev.key; ch = ev.scan;
                }
                calc_render_window(cx, cy, cw, ch, &state);
            } else if (ev.type == EVENT_CLICK) {
                int mouse_x = ev.x;
                int mouse_y = ev.y;

                for (int r = 0; r < 5; r++) {
                    for (int c = 0; c < 4; c++) {
                        int bx = cx + 16 + c * 80;
                        int by = cy + 90 + r * 56;
                        if (mouse_x >= bx && mouse_x <= bx + 68 && mouse_y >= by && mouse_y <= by + 50) {
                            if (r == 0 && c == 0) calc_clear_all(&state);
                            else if (r == 0 && c == 1) calc_clear_all(&state);
                            else if (r == 0 && c == 2) {
                                int len = strlen(state.entry_buf);
                                if (len > 1) {
                                    state.entry_buf[len - 1] = '\0';
                                } else {
                                    state.entry_buf[0] = '0';
                                    state.entry_buf[1] = '\0';
                                    state.is_new_entry = 1;
                                }
                            }
                            else if (r == 0 && c == 3) calc_input_op(&state, '/');
                            else if (r == 1 && c == 0) calc_input_digit(&state, 7);
                            else if (r == 1 && c == 1) calc_input_digit(&state, 8);
                            else if (r == 1 && c == 2) calc_input_digit(&state, 9);
                            else if (r == 1 && c == 3) calc_input_op(&state, '*');
                            else if (r == 2 && c == 0) calc_input_digit(&state, 4);
                            else if (r == 2 && c == 1) calc_input_digit(&state, 5);
                            else if (r == 2 && c == 2) calc_input_digit(&state, 6);
                            else if (r == 2 && c == 3) calc_input_op(&state, '-');
                            else if (r == 3 && c == 0) calc_input_digit(&state, 1);
                            else if (r == 3 && c == 1) calc_input_digit(&state, 2);
                            else if (r == 3 && c == 2) calc_input_digit(&state, 3);
                            else if (r == 3 && c == 3) calc_input_op(&state, '+');
                            else if (r == 4 && c == 0) {
                                if (state.entry_buf[0] == '-') {
                                    int l = strlen(state.entry_buf);
                                    for (int i = 0; i < l; i++) {
                                        state.entry_buf[i] = state.entry_buf[i + 1];
                                    }
                                } else if (state.entry_buf[0] != '0') {
                                    int l = strlen(state.entry_buf);
                                    if (l < 14) {
                                        for (int i = l; i >= 0; i--) {
                                            state.entry_buf[i + 1] = state.entry_buf[i];
                                        }
                                        state.entry_buf[0] = '-';
                                    }
                                }
                            }
                            else if (r == 4 && c == 1) calc_input_digit(&state, 0);
                            else if (r == 4 && c == 2) {
                                int has_dot = 0;
                                for (int i = 0; state.entry_buf[i]; i++) {
                                    if (state.entry_buf[i] == '.') has_dot = 1;
                                }
                                if (!has_dot) {
                                    int len = strlen(state.entry_buf);
                                    if (len < 14) {
                                        state.entry_buf[len] = '.';
                                        state.entry_buf[len + 1] = '\0';
                                    }
                                }
                            }
                            else if (r == 4 && c == 3) calc_input_equals(&state);
                            maxos_play_sound(800, 15);
                            calc_render_lcd(cx, cy, cw, state.entry_buf);
                        }
                    }
                }
            } else if (ev.type == EVENT_KEY) {
                char ch = (char)ev.key;
                if (ch >= '0' && ch <= '9') {
                    calc_input_digit(&state, ch - '0');
                    maxos_play_sound(800, 15);
                    calc_render_lcd(cx, cy, cw, state.entry_buf);
                } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                    calc_input_op(&state, ch);
                    maxos_play_sound(900, 15);
                    calc_render_lcd(cx, cy, cw, state.entry_buf);
                } else if (ch == '=' || ch == '\n' || ev.scan == 0x1C) {
                    calc_input_equals(&state);
                    maxos_play_sound(1000, 20);
                    calc_render_lcd(cx, cy, cw, state.entry_buf);
                } else if (ch == 'c' || ch == 'C' || ev.scan == 0x53) {
                    calc_clear_all(&state);
                    calc_render_lcd(cx, cy, cw, state.entry_buf);
                }
            }
        }
        maxos_yield();
    }
}

// Stubs for maxp legacy code
void calc_init(void) {}
void calc_open_window(void) { maxp_spawn_instance(MAXP_APP_CALC, "Calculator", 0); }
void calc_close_window(void) {}
void calc_draw(void) {}
int calc_handle_click(int mx, int my) { return 0; }
int calc_handle_key(char ch, unsigned char scan) { return 0; }

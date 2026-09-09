#ifndef CALC_H
#define CALC_H

extern int calc_open;

typedef struct {
    char entry_buf[32];
    double stored_val;
    char pending_op;
    int is_new_entry;
    int is_error;
} calc_state_t;

void calc_init(void);
void calc_instance_init(calc_state_t* s);
void calc_instance_draw(calc_state_t* s, int cx, int cy, int cw, int ch);
int calc_instance_click(calc_state_t* s, int cx, int cy, int cw, int ch, int mx, int my);
int calc_instance_key(calc_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy wrappers
void calc_open_window(void);
void calc_close_window(void);
void calc_draw(void);
int calc_handle_click(int mouse_x, int mouse_y);
int calc_handle_key(char ascii_char, unsigned char scan_code);

#endif // CALC_H

#ifndef CALC_H
#define CALC_H

extern int calc_open;

void calc_init(void);
void calc_open_window(void);
void calc_close_window(void);
void calc_draw(void);
int calc_handle_click(int mouse_x, int mouse_y);
int calc_handle_key(char ascii_char, unsigned char scan_code);

#endif // CALC_H

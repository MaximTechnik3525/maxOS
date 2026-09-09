#ifndef PONG_H
#define PONG_H

extern int pong_open;

void pong_init(void);
void pong_open_window(void);
void pong_close_window(void);
void pong_draw(void);
void pong_tick(void);
int pong_handle_click(int mouse_x, int mouse_y);
int pong_handle_key(char ascii_char, unsigned char scan_code);

#endif // PONG_H

#ifndef EXPLORER_H
#define EXPLORER_H

#include "maxfs.h"

extern int explorer_open;

void explorer_init(void);
void explorer_open_window(void);
void explorer_close_window(void);
void explorer_draw(void);
int explorer_handle_click(int mouse_x, int mouse_y);
int explorer_handle_key(char ascii_char, unsigned char scan_code);

#endif // EXPLORER_H

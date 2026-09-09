#ifndef NOTEPAD_H
#define NOTEPAD_H

#include "maxfs.h"

extern int notepad_open;
extern char ftext[1024];
extern int textid;
extern char fname_input[32];
extern int fname_len;

void notepad_init(void);
void notepad_open_window(void);
void notepad_open_file_by_name(const char* name);
void notepad_close_window(void);
void notepad_draw(void);
void notepad_draw_text(void);
int notepad_handle_click(int mouse_x, int mouse_y);
int notepad_handle_key(char ascii_char, unsigned char scan_code);

#endif // NOTEPAD_H

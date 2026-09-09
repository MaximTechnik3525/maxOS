#ifndef EXPLORER_H
#define EXPLORER_H

#include "maxfs.h"

extern int explorer_open;

typedef struct {
    int selected_file;
    char exp_status[64];
} explorer_state_t;

void explorer_init(void);
void explorer_instance_init(explorer_state_t* s);
void explorer_instance_draw(explorer_state_t* s, int exp_x, int exp_y, int exp_w, int exp_h);
int explorer_instance_click(explorer_state_t* s, int exp_x, int exp_y, int exp_w, int exp_h, int mouse_x, int mouse_y);
int explorer_instance_key(explorer_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy wrappers
void explorer_open_window(void);
void explorer_close_window(void);
void explorer_draw(void);
int explorer_handle_click(int mouse_x, int mouse_y);
int explorer_handle_key(char ascii_char, unsigned char scan_code);

#endif // EXPLORER_H

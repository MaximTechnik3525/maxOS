#ifndef NOTEPAD_H
#define NOTEPAD_H

#include "maxfs.h"

extern int notepad_open;
extern char ftext[1024];
extern int textid;
extern char fname_input[32];
extern int fname_len;

typedef struct {
    char ftext[2048];
    int textid;
    char fname_input[32];
    int fname_len;
    char np_status[64];
    int current_color_idx;
    int focus_mode;
    int file_picker_open;
    int picker_selected;
} notepad_state_t;

void notepad_init(void);
void notepad_instance_init(notepad_state_t* s, const char* initial_file);
void notepad_instance_draw(notepad_state_t* s, int nx, int ny, int nw, int nh);
int notepad_instance_click(notepad_state_t* s, int nx, int ny, int nw, int nh, int mx, int my);
int notepad_instance_key(notepad_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy wrappers
void notepad_open_window(void);
void notepad_open_file_by_name(const char* name);
void notepad_close_window(void);
void notepad_draw(void);
void notepad_draw_text(void);
int notepad_handle_click(int mouse_x, int mouse_y);
int notepad_handle_key(char ascii_char, unsigned char scan_code);

// Event Loop Entry Point (v4.0 EventUpdate)
void notepad_main(void);

#endif // NOTEPAD_H

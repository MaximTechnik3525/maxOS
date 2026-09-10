#ifndef EXPLORER_H
#define EXPLORER_H

#include "maxfs.h"
#include "dialog.h"

#define EXPLORER_MAX_VISIBLE 16

#define EXPLORER_DLG_NEW_FILE 1
#define EXPLORER_DLG_NEW_DIR  2
#define EXPLORER_DLG_RENAME   3

extern int explorer_open;

typedef struct {
    int current_dir_inode;    // 0 = root (/), or inode index of current directory
    int selected_index;       // index in current directory item list (0..total_items-1), or -1
    int scroll_offset;        // index of first visible row in table
    char exp_status[80];      // status bar text
    dialog_t dlg;             // dialog window for filename assignment and rename
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

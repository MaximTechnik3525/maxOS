#ifndef INSTALLER_H
#define INSTALLER_H

extern int installer_open;

typedef struct {
    int install_progress;
    char install_status[64];
} installer_state_t;

void installer_instance_init(installer_state_t* s);
void installer_instance_draw(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h);
int  installer_instance_click(installer_state_t* s, int inst_x, int inst_y, int inst_w, int inst_h, int mouse_x, int mouse_y);
int  installer_instance_key(installer_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy API Wrappers
void installer_init(void);
void installer_open_window(void);
void installer_close_window(void);
void installer_draw(void);
int  installer_handle_click(int mouse_x, int mouse_y);
int  installer_handle_key(char ascii_char, unsigned char scan_code);

// Event Loop Entry Point (v4.0 EventUpdate)
void installer_main(void);

#endif // INSTALLER_H

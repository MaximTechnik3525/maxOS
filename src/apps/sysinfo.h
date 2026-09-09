#ifndef SYSINFO_H
#define SYSINFO_H

extern int sysinfo_open;

typedef struct {
    int dummy;
} sysinfo_state_t;

void sysinfo_init(void);
void sysinfo_instance_init(sysinfo_state_t* s);
void sysinfo_instance_draw(sysinfo_state_t* s, int sx, int sy, int sw, int sh);
int sysinfo_instance_click(sysinfo_state_t* s, int sx, int sy, int sw, int sh, int mouse_x, int mouse_y);
int sysinfo_instance_key(sysinfo_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy wrappers
void sysinfo_open_window(void);
void sysinfo_close_window(void);
void sysinfo_draw(void);
int sysinfo_handle_click(int mouse_x, int mouse_y);
int sysinfo_handle_key(char ascii_char, unsigned char scan_code);

#endif // SYSINFO_H

#ifndef SYSINFO_H
#define SYSINFO_H

extern int sysinfo_open;

void sysinfo_init(void);
void sysinfo_open_window(void);
void sysinfo_close_window(void);
void sysinfo_draw(void);
int sysinfo_handle_click(int mouse_x, int mouse_y);
int sysinfo_handle_key(char ascii_char, unsigned char scan_code);

#endif // SYSINFO_H

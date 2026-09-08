#ifndef INSTALLER_H
#define INSTALLER_H

extern int installer_open;

void installer_init(void);
void installer_open_window(void);
void installer_close_window(void);
void installer_draw(void);
int installer_handle_click(int mouse_x, int mouse_y);
int installer_handle_key(char ascii_char, unsigned char scan_code);

#endif // INSTALLER_H

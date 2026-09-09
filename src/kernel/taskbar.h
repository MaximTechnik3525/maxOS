#ifndef TASKBAR_H
#define TASKBAR_H

#define TASKBAR_Y 730
#define TASKBAR_HEIGHT 38

extern int start_menu_open;

void taskbar_init(void);
void taskbar_draw(void);
void taskbar_draw_desktop_icons(void);
void taskbar_draw_clock(void);
int taskbar_is_start_menu_open(void);
void taskbar_toggle_start_menu(void);
void taskbar_close_start_menu(void);
int taskbar_handle_click(int mouse_x, int mouse_y);
int taskbar_handle_key(char ascii_char, unsigned char scan_code);
int taskbar_is_app_minimized(void);
void taskbar_set_app_minimized(int min);

#endif // TASKBAR_H

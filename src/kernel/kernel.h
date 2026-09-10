#ifndef KERNEL_H
#define KERNEL_H

// Display dimensions
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// Global graphics memory framebuffer (16-bit RGB 565)
extern unsigned short* _gfx_memory_backend;
#define gfx_memory _gfx_memory_backend

// Window & Desktop coordinates
extern int win_x, win_y, win_w, win_h;
extern int pos_x, pos_y;
extern int theme;
extern int drag;

// Graphics primitives
void draw_char(char c, int start_x, int start_y, unsigned short color);
void print_string(char* str, int x, int y, unsigned short color);
void draw_rect(int rx, int ry, int rw, int rh, unsigned short color);
void draw_cursor(int mouse_x, int mouse_y);
void prev_cursor(void);
void draw_window(void);

// Hardware Port I/O
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short val);

// System timing and sound
void sleep(unsigned int ms);
void play_sound(unsigned int nfreq);
void no_sound(void);

// System control
void shutdown(void);
void system_reboot(void);
void error(char* err);

#include "string.h"

// Utilities
void int_str(int num, char* str);
int str_in(char* main_string, char* substring);
char scan_code_to_ascii(unsigned char scan_code);

// UI Primitives
void draw_3d_box(int bx, int by, int bw, int bh, int sunken, unsigned short fill);
void draw_ui_btn_state(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col, int pressed);
void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col);
void ui_btn_click_effect(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col);
void draw_ui_button(int x, int y, int w, int h, const char* text, unsigned short bg_col, unsigned short text_col, int sunken);
void ui_button_click_effect(int x, int y, int w, int h, const char* text, unsigned short bg_col, unsigned short text_col);
int desktop_handle_click(int mouse_x, int mouse_y);
void pump_events_nonblocking(void);

// Memory Statistics for System & Mem App
struct SystemMemInfo {
    unsigned int total_kb;
    unsigned int used_kb;
    unsigned int free_kb;
    unsigned int lower_kb;
    unsigned int upper_kb;
    unsigned int kernel_kb;
    unsigned int vram_kb;
    unsigned int paging_kb;
    unsigned int stacks_kb;
    unsigned int ramdisk_kb;
    unsigned int apps_dynamic_kb;
    unsigned int stress_kb;
    unsigned int usage_percent;
};

void get_system_mem_info(struct SystemMemInfo* info);

#endif // KERNEL_H

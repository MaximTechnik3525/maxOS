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
void error(char* err);

// Utilities
void int_str(int num, char* str);
int str_in(char* main_string, char* substring);
char scan_code_to_ascii(unsigned char scan_code);

#endif // KERNEL_H

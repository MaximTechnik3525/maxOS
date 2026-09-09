#ifndef MEM_H
#define MEM_H

extern int mem_open;

void mem_init(void);
void mem_open_window(void);
void mem_close_window(void);
void mem_draw(void);
int mem_handle_click(int mouse_x, int mouse_y);
int mem_handle_key(char ascii_char, unsigned char scan_code);

#endif // MEM_H

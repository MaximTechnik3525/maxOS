#ifndef MEM_H
#define MEM_H

typedef struct {
    int mem_view_mode;     // 0 = RAM Monitor, 1 = Task Manager, 2 = RAM Stress Test
    int selected_task_idx;
    int optimize_flash;
} mem_state_t;

extern int mem_open;

void mem_init(void);
void mem_instance_init(mem_state_t* s, int mode);
void mem_instance_draw(mem_state_t* s, int sx, int sy, int sw, int sh);
int mem_instance_click(mem_state_t* s, int sx, int sy, int sw, int sh, int mouse_x, int mouse_y);
int mem_instance_key(mem_state_t* s, char ascii_char, unsigned char scan_code);
void mem_instance_tick(mem_state_t* s, int sx, int sy, int sw, int sh);

// Legacy wrappers
void mem_open_window(void);
void mem_open_stress_window(void);
void mem_close_window(void);
void mem_draw(void);
int mem_handle_click(int mouse_x, int mouse_y);
int mem_handle_key(char ascii_char, unsigned char scan_code);
void mem_tick(void);

// RAM Stress Test Engine & Diagnostics
unsigned int mem_get_stress_kb(void);
int mem_stress_is_active(void);
void mem_stress_run_pass(void);
void mem_stress_release(void);

// Event Loop Entry Point (v4.0 EventUpdate)
void mem_main(void);

#endif // MEM_H

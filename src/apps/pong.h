#ifndef PONG_H
#define PONG_H

extern int pong_open;

typedef struct {
    int pad_x;
    int pad_y;
    int pad_w;
    int pad_h;
    int ball_x;
    int ball_y;
    int ball_dx;
    int ball_dy;
    int ball_size;
    int pong_score;
    int pong_high_score;
    int pong_game_over;
} pong_state_t;

void pong_init(void);
void pong_instance_init(pong_state_t* s, int px, int py, int pw, int ph);
void pong_instance_draw(pong_state_t* s, int px, int py, int pw, int ph);
void pong_instance_tick(pong_state_t* s, int px, int py, int pw, int ph);
int pong_instance_click(pong_state_t* s, int px, int py, int pw, int ph, int mx, int my);
int pong_instance_key(pong_state_t* s, char ascii_char, unsigned char scan_code);

// Legacy wrappers
void pong_open_window(void);
void pong_close_window(void);
void pong_draw(void);
void pong_tick(void);
int pong_handle_click(int mouse_x, int mouse_y);
int pong_handle_key(char ascii_char, unsigned char scan_code);

// Event Loop Entry Point (v4.0 EventUpdate)
void pong_main(void);

#endif // PONG_H

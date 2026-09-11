#ifndef MAXOS_H
#define MAXOS_H

// maxOS specific GUI and OS API
#define EVENT_NONE  0
#define EVENT_CLICK 1
#define EVENT_KEY   2
#define EVENT_DRAW  3
#define EVENT_TICK  4
#define EVENT_CLOSE 5

typedef struct {
    int type;
    int x;
    int y;
    int key;
    int scan;
} maxos_event_t;

void maxos_print_text(const char* text, int x, int y, unsigned short color);
void maxos_draw_rect(int x, int y, int w, int h, unsigned short color);
void maxos_play_sound(unsigned int freq, unsigned int ms);
void maxos_sleep(unsigned int ms);
void maxos_yield(void);
void maxos_draw_window(void);
void maxos_debug_log(const char* tag, const char* msg);
int maxos_get_event(maxos_event_t* ev);

#endif

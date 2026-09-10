#ifndef EVENT_H
#define EVENT_H

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
} kernel_event_t;

#define MAX_TASK_EVENTS 32

typedef struct {
    kernel_event_t events[MAX_TASK_EVENTS];
    int head;
    int tail;
} event_queue_t;

void event_push(event_queue_t* q, kernel_event_t ev);
int event_pop(event_queue_t* q, kernel_event_t* ev);

#endif

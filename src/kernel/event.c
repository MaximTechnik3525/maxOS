#include "event.h"

void event_push(event_queue_t* q, kernel_event_t ev) {
    int next = (q->tail + 1) % MAX_TASK_EVENTS;
    if (next != q->head) {
        q->events[q->tail] = ev;
        q->tail = next;
    }
}

int event_pop(event_queue_t* q, kernel_event_t* ev) {
    if (q->head == q->tail) return 0;
    *ev = q->events[q->head];
    q->head = (q->head + 1) % MAX_TASK_EVENTS;
    return 1;
}

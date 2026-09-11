#include "include/stdlib.h"
#include "include/stdio.h"
#include "include/string.h"
#include "include/maxos.h"
#include "sys/syscall.h"

// -----------------------------------------------------------------------------
// STDLIB
// -----------------------------------------------------------------------------
void* malloc(size_t size) {
    return u_malloc(size);
}

void free(void* ptr) {
    u_free(ptr);
}

void exit(int status) {
    (void)status;
    u_exit();
    while (1) {}
}

// -----------------------------------------------------------------------------
// MAXOS API
// -----------------------------------------------------------------------------
void maxos_print_text(const char* text, int x, int y, unsigned short color) {
    u_print_string(text, x, y, color);
}

void maxos_draw_rect(int x, int y, int w, int h, unsigned short color) {
    u_draw_rect(x, y, w, h, color);
}

void maxos_play_sound(unsigned int freq, unsigned int ms) {
    u_play_sound(freq, ms);
}

void maxos_sleep(unsigned int ms) {
    u_sleep(ms);
}

void maxos_yield(void) {
    u_yield();
}

void maxos_draw_window(void) {
    u_draw_window();
}

void maxos_debug_log(const char* tag, const char* msg) {
    u_debug_log(tag, msg);
}

int maxos_get_event(maxos_event_t* ev) {
    return u_get_event(ev);
}

int maxos_pci_count(void) {
    return u_pci_count();
}

int maxos_pci_get_device(int index, void* out_dev) {
    return u_pci_get_dev(index, out_dev);
}


// -----------------------------------------------------------------------------
// STDIO (Simple wrapper over maxos_debug_log for now)
// -----------------------------------------------------------------------------
int puts(const char* s) {
    maxos_debug_log("APP", s);
    return 0;
}

int putchar(int c) {
    char buf[2] = {(char)c, 0};
    maxos_debug_log("APP", buf);
    return c;
}


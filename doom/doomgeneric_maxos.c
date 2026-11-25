#include "../maxos_api.h"
#include "doomgeneric.h"
#include "doomkeys.h"
#include <stddef.h>
#include <stdarg.h>
static maxos_api_t* sys_api;
static unsigned int ticks_ms = 0;
static int doom_running = 1;
void DG_Init() {
    ticks_ms = 0;
    doom_running = 1;
}
void DG_DrawFrame() {
    if (!DG_ScreenBuffer || !sys_api) { return; }
    int offset_y = 60;
    for (int y = 0; y < DOOMGENERIC_RESY; y++) {
        int dst_y1 = offset_y + (y * 3) / 2;
        int dst_y2 = offset_y + ((y + 1) * 3) / 2;
        if (dst_y2 > sys_api->screen_h) { dst_y2 = sys_api->screen_h; }
        for (int x = 0; x < DOOMGENERIC_RESX; x++) {
            unsigned int pixel = DG_ScreenBuffer[y * DOOMGENERIC_RESX + x];
            unsigned short r = ((pixel >> 16) & 0xFF) >> 3;
            unsigned short g = ((pixel >> 8) & 0xFF) >> 2;
            unsigned short b = (pixel & 0xFF) >> 3;
            unsigned short rgb565 = (r << 11) | (g << 5) | b;
            int dst_x = x * 2;
            for (int dy = dst_y1; dy < dst_y2; dy++) {
                sys_api->back_buffer[dy * sys_api->screen_w + dst_x] = rgb565;
                sys_api->back_buffer[dy * sys_api->screen_w + dst_x + 1] = rgb565;
            }
        }
    }
    sys_api->update_screen();
    sys_api->sleep(26);
    ticks_ms += 28;
}
void DG_SleepMs(uint32_t ms) {
    sys_api->sleep(ms);
    ticks_ms += ms;
}
uint32_t DG_GetTicksMs() {
    return ticks_ms;
}
static unsigned char scancode_to_doom(unsigned char sc) {
    switch (sc) {
        case 0x1C: return KEY_ENTER;
        case 0x01: return KEY_ESCAPE;
        case 0x4B: case 0x1E: return KEY_LEFTARROW;
        case 0x4D: case 0x20: return KEY_RIGHTARROW;
        case 0x48: case 0x11: return KEY_UPARROW;
        case 0x50: case 0x1F: return KEY_DOWNARROW;
        case 0x1D: return KEY_FIRE;
        case 0x39: return KEY_USE;
        case 0x2A: case 0x36: return KEY_RSHIFT;
        case 0x0F: return KEY_TAB;
        case 0x15: return 'y';
        case 0x31: return 'n';
        default: return 0;
    }
}
int DG_GetKey(int* pressed, unsigned char* doomKey) {
    unsigned char status = sys_api->inb(0x64);
    if (status & 0x01) {
        if (status & 0x20) {
            sys_api->inb(0x60);
            return 0;
        }
        unsigned char sc = sys_api->inb(0x60);
        if (sc == 0x58) {
            doom_running = 0;
            return 0;
        }
        int is_pressed = (sc < 0x80) ? 1 : 0;
        unsigned char clean_sc = sc & 0x7F;
        unsigned char dk = scancode_to_doom(clean_sc);
        if (dk != 0) {
            if (is_pressed == 1) {
                if (dk == KEY_FIRE) {
                    sys_api->play_sound(950); sys_api->sleep(4);
                    sys_api->play_sound(320); sys_api->sleep(6);
                    unsigned int noise = 0xACE1;
                    for (int i = 0; i < 18; i++) {
                        noise = noise * 1103515245 + 12345;
                        unsigned int base_freq = 450 - (i * 18);
                        unsigned int freq = base_freq + ((noise >> 16) % (2600 - i * 110));
                        sys_api->play_sound(freq);
                        sys_api->sleep(2);
                    }
                    sys_api->no_sound();
                }
                else if (dk == KEY_USE) {
                    sys_api->play_sound(480); sys_api->sleep(8);
                    sys_api->play_sound(820); sys_api->sleep(12);
                    sys_api->no_sound();
                }
            }
            *pressed = is_pressed;
            *doomKey = dk;
            return 1;
        }
    }
    return 0;
}
void DG_SetWindowTitle(const char* title) { (void)title; }
static unsigned char* heap_ptr = (unsigned char*)0x4000000;
void* malloc(size_t size) {
    size = (size + 15) & ~15;
    void* p = heap_ptr;
    heap_ptr += size;
    return p;
}
void free(void* ptr) { (void)ptr; }
void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    unsigned char* p = (unsigned char*)malloc(total);
    for (size_t i  = 0; i < total; i++) {
        p[i] = 0;
    }
    return p;
}
void* realloc(void* ptr, size_t size) {
    void* new_p = malloc(size);
    if (ptr && new_p) {
        unsigned char* s = (unsigned char*)ptr;
        unsigned char* d = (unsigned char*)new_p;
        for (size_t i = 0; i < size; i++) {
            d[i] = s[i];
        }
    }
    return new_p;
}
typedef struct {
    unsigned char* data;
    unsigned int size;
    unsigned int pos;
    int in_use;
} MEM_FILE;
static MEM_FILE wad_file;
void* fopen(const char* filename, const char* mode) {
    (void)mode;
    if (!sys_api || !sys_api->wad_addr) {
        return NULL;
    }
    const char* p = filename;
    int is_wad = 0;
    while (*p) {
        if ((*p == 'w' || *p == 'W') && (*(p+1) == 'a' || *(p+1) == 'A') && (*(p+2) == 'd' || *(p+2) == 'D')) {
            is_wad = 1;
            break;
        }
        p++;
    }
    if (!is_wad) { return NULL; }
    wad_file.data = sys_api->wad_addr;
    wad_file.size = sys_api->wad_size;
    wad_file.pos = 0;
    wad_file.in_use = 1;
    return &wad_file;
}
int fclose(void* stream) {
    if (stream == &wad_file) {
        wad_file.in_use = 0;
    }
    return 0;
}
size_t fread(void* ptr, size_t size, size_t nmemb, void* stream) {
    if (stream != &wad_file || !wad_file.in_use) {
        return 0;
    }
    size_t total = size * nmemb;
    if (wad_file.pos + total > wad_file.size) {
        total = wad_file.size - wad_file.pos;
    }
    unsigned char* dst = (unsigned char*)ptr;
    for (size_t i = 0; i < total; i++) {
        dst[i] = wad_file.data[wad_file.pos + i];
    }
    wad_file.pos += total;
    return total / size;
}
size_t fwrite(const void* ptr, size_t size, size_t nmemb, void* stream) {
    (void)ptr; (void)size; (void)nmemb; (void)stream;
    return 0;
}
int fseek(void* stream, long offset, int whence) {
    if (stream != &wad_file || !wad_file.in_use) {
        return -1;
    }
    long new_pos = 0;
    if (whence == 0) {
        new_pos = offset;
    }
    else if (whence == 1) {
        new_pos = (long)wad_file.pos + offset;
    }
    else if (whence == 2) {
        new_pos = (long)wad_file.size + offset;
    }
    if (new_pos < 0) {
        new_pos = 0;
    }
    if ((unsigned int)new_pos > wad_file.size) {
        new_pos = wad_file.size;
    }
    wad_file.pos = (unsigned int)new_pos;
    return 0;
}
long ftell(void* stream) {
    if (stream != &wad_file || !wad_file.in_use) {
        return -1;
    }
    return (long)wad_file.pos;
}
int fflush(void* stream) { (void)stream; return 0; }
int remove(const char* path) { (void)path; return 0; }
int rename(const char* oldp, const char* newp) { (void)oldp; (void)newp; return 0; }
int mkdir(const char* path, unsigned int mode) { (void)path; (void)mode; return 0; }
int printf(const char* fmt, ...) { (void)fmt; return 0; }
int puts(const char* s) { (void)s; return 0; }
int putchar(int c) { return c; }
void exit(int status) { (void)status; doom_running = 0; while (1); }
static int err_y = 80;
int fprintf(void* stream, const char* fmt, ...) {
    (void)stream;
    if (sys_api && fmt) {
        sys_api->print_string((char*)fmt, 20, err_y, 0xF800);
        err_y += 16;
        sys_api->update_screen();
    }
    return 0;
}
extern int vsnprintf(char* str, size_t size, const char* format, va_list ap);
int vfprintf(void* stream, const char* fmt, va_list ap) {
    (void)stream;
    if (sys_api && fmt) {
        char buf[256];
        vsnprintf(buf, sizeof(buf), fmt, ap);
        sys_api->print_string(buf, 20, err_y, 0xF800);
        err_y += 16;
        sys_api->update_screen();
    }
    return 0;
}
void _start(maxos_api_t* api) {
    sys_api = api;
    heap_ptr = (unsigned char*)0x4000000;
    // for (int y = 0; y < api->screen_h; y++) {
    //     for (int x = 0; x < api->screen_w; x++) {
    //         api->back_buffer[y * api->screen_w + x] = 0x0000;
    //     }
    // }
    api->print_string("Loading DOOM (Shareware)...", 20, 20, 0xF800);
    api->print_string("Controls: WASD/ARROWS - move, CTRL - fire, SPACE - use, F12 - exit to maxOS.", 20, 40, 0xFFFF);
    api->update_screen();
        char dbg[64];
    extern int snprintf(char* str, size_t size, const char* format, ...);
    if (api->wad_addr && api->wad_size > 12) {
        snprintf(dbg, sizeof(dbg), "WAD Header: %c%c%c%c | Size: %d bytes",
                 api->wad_addr[0], api->wad_addr[1], api->wad_addr[2], api->wad_addr[3], api->wad_size);
        api->print_string(dbg, 20, 60, 0x07E0);
    } else {
        api->print_string("ERROR: WAD NOT LOADED BY GRUB!", 20, 60, 0xF800);
    }
    api->update_screen();
    char* argv[] = {"doom", "-iwad", "doom1.wad"};
    doomgeneric_Create(3, argv);
    while (doom_running) {
        doomgeneric_Tick();
    }
}
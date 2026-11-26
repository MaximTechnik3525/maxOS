#include "../maxos_api.h"
#include "doomgeneric.h"
#include "doomkeys.h"
#include <stddef.h>
#include <stdarg.h>
#include "i_sound.h"
#include "w_wad.h"
static maxos_api_t* sys_api;
static unsigned int ticks_ms = 0;
static int doom_running = 1;
static snddevice_t maxos_devs[] = { SNDDEVICE_SB, SNDDEVICE_PCSPEAKER };
static boolean I_MaxOS_Init(boolean p) { (void)p; return true; }
static void I_MaxOS_Shut(void) { if (sys_api) sys_api->no_sound(); }
static int I_MaxOS_Lump(sfxinfo_t *sfx) {
    char name[16];
    extern int snprintf(char* s, size_t n, const char* f, ...);
    snprintf(name, 16, "dp%s", sfx->name);
    return W_CheckNumForName(name);
}
static void I_MaxOS_Upd(void) {}
static void I_MaxOS_Par(int c, int v, int s) { (void)c, (void)v, (void)s; }
static boolean I_MaxOS_Play(int c) { (void)c; return false; }
static int I_MaxOS_Start(sfxinfo_t *sfx, int ch, int vol, int sep) {
    (void)vol; (void)sep;
    if (!sys_api || !sfx || sfx->lumpnum < 0) {
        return -1;
    }
    int len = W_LumpLength(sfx->lumpnum);
    const unsigned char* d = (const unsigned char*)W_CacheLumpNum(sfx->lumpnum, 1);
    if (!d || len <= 8) return -1;
    if (d[0] == 0 && d[1] == 0) {
        int count = d[2] | (d[3] << 8);
        if (count > len - 4) count = len - 4;
        int step = (count > 12) ? (count / 12) : 1;
        for (int i = 0; i < count; i += step) {
            unsigned char t = d[4 + i];
            if (t == 0) sys_api->no_sound();
            else sys_api->play_sound(140 + t * 32);
            sys_api->sleep(2);
        }
    } else if (d[0] == 3 && d[1] == 0) {
        int count = len - 8;
        int step = (count > 14) ? (count / 14) : 1;
        for (int i = 0; i < count; i += step) {
            int amp = (int)d[8 + i] - 128;
            if (amp < 0) amp = -amp;
            if (amp < 6) sys_api->no_sound();
            else sys_api->play_sound(120 + amp * 16);
            sys_api->sleep(2);
        }
    }
    sys_api->no_sound();
    return ch;
}
sound_module_t maxos_pcspeaker_module = {
    maxos_devs, 2, I_MaxOS_Init, I_MaxOS_Shut, I_MaxOS_Lump,
    I_MaxOS_Upd, I_MaxOS_Par, I_MaxOS_Start, I_MaxOS_Upd, I_MaxOS_Play, NULL
};
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
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';

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
FILE* fopen(const char* filename, const char* mode) {
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
    return (FILE*)&wad_file;
}

int fclose(FILE* stream) {
    if (stream == (FILE*)&wad_file) {
        wad_file.in_use = 0;
    }
    return 0;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (stream != (FILE*)&wad_file || !wad_file.in_use) {
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

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    (void)ptr; (void)size; (void)nmemb; (void)stream;
    return 0;
}

int fseek(FILE* stream, long offset, int whence) {
    if (stream != (FILE*)&wad_file || !wad_file.in_use) {
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

long ftell(FILE* stream) {
    if (stream != (FILE*)&wad_file || !wad_file.in_use) {
        return -1;
    }
    return (long)wad_file.pos;
}

int fflush(FILE* stream) { (void)stream; return 0; }
int remove(const char* path) { (void)path; return 0; }
int rename(const char* oldp, const char* newp) { (void)oldp; (void)newp; return 0; }
int mkdir(const char* path, unsigned int mode) { (void)path; (void)mode; return 0; }
int printf(const char* fmt, ...) { (void)fmt; return 0; }
int puts(const char* s) { (void)s; return 0; }
int putchar(int c) { return c; }
void exit(int status) { (void)status; doom_running = 0; while (1); }
static int err_y = 80;

int fprintf(FILE* stream, const char* fmt, ...) {
    (void)stream;
    if (sys_api && fmt) {
        sys_api->print_string((char*)fmt, 20, err_y, 0xF800);
        err_y += 16;
        sys_api->update_screen();
    }
    return 0;
}

extern int vsnprintf(char* str, size_t size, const char* format, va_list ap);
int vfprintf(FILE* stream, const char* fmt, va_list ap) {
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
    api->print_string("Loading DOOM (Shareware)...", 20, 20, 0xF800);
    api->print_string("Controls: [WASD/ARROWS] - Move, [CTRL] - Fire, [SPACE] - Use, [1-7] - Change weapon, [F12] - Exit to maxOS.", 20, 40, 0xFFFF);
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
    char* argv[] = {"doom", "-iwad", "doom1.wad", "-sfxvolume", "15"};
    doomgeneric_Create(5, argv);
    while (doom_running) {
        doomgeneric_Tick();
    }
}
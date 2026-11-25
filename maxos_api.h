#ifndef MAXOS_API_H
#define MAXOS_API_H
typedef struct {
    unsigned short* back_buffer;
    int screen_w;
    int screen_h;
    void (*update_screen)();
    void (*print_string)(char* str, int x, int y, unsigned short color);
    void (*sleep)(unsigned int ms);
    void (*play_sound)(unsigned int nfreq);
    void (*no_sound)();
    unsigned char (*inb)(unsigned short port);
    void (*outb)(unsigned short port, unsigned char data);
    unsigned char* wad_addr;
    unsigned int wad_size;
} maxos_api_t;
#endif
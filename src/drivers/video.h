#ifndef VIDEO_H
#define VIDEO_H

#include "../kernel/font.h"

// Pixel format flags
#define VIDEO_FORMAT_UNKNOWN  0
#define VIDEO_FORMAT_RGB565   1 // 16-bit (VBE standard)
#define VIDEO_FORMAT_BGR565   2 // 16-bit BGR
#define VIDEO_FORMAT_RGB888   3 // 24-bit RGB
#define VIDEO_FORMAT_BGR888   4 // 24-bit BGR
#define VIDEO_FORMAT_ARGB8888 5 // 32-bit ARGB
#define VIDEO_FORMAT_BGRA8888 6 // 32-bit BGRA (UEFI GOP standard)
#define VIDEO_FORMAT_RGBA8888 7 // 32-bit RGBA

typedef struct {
    unsigned char* fb;             // Framebuffer linear address
    unsigned int width;            // Width in pixels (e.g. 1024)
    unsigned int height;           // Height in pixels (e.g. 768)
    unsigned int pitch;            // Bytes per scanline
    unsigned char bpp;             // Bits per pixel (16, 24, 32)
    unsigned char format;          // VIDEO_FORMAT_*
    unsigned char red_pos;
    unsigned char red_size;
    unsigned char green_pos;
    unsigned char green_size;
    unsigned char blue_pos;
    unsigned char blue_size;
    int is_uefi;                   // 1 if UEFI GOP, 0 if Legacy BIOS VBE
    const char* mode_name;         // "UEFI GOP" or "BIOS VBE"
} video_driver_t;

extern video_driver_t g_video;

// Initialization from Multiboot Info
void video_init(void* multiboot_info_ptr);

// Status queries
int video_is_uefi(void);
int video_get_width(void);
int video_get_height(void);
int video_get_bpp(void);
int video_get_pitch(void);
const char* video_get_mode_name(void);

// Color conversion
unsigned int video_rgb565_to_native(unsigned short c565);
unsigned short video_native_to_rgb565(unsigned int native_col);

// Drawing primitives
void video_put_pixel(int x, int y, unsigned short rgb565);
unsigned short video_get_pixel(int x, int y);
void video_draw_rect(int x, int y, int w, int h, unsigned short rgb565);
void video_draw_char(char c, int x, int y, unsigned short rgb565);
void video_print_string(const char* str, int x, int y, unsigned short rgb565);
void video_clear(unsigned short rgb565);
void video_scroll(int x, int y, int w, int h, int lines, unsigned short fill_color);
void video_render_wallpaper(unsigned short c1, unsigned short c2);

// Cursor support (saves & restores underlying pixels regardless of bpp)
void video_draw_cursor(int mouse_x, int mouse_y, unsigned short color1, unsigned short color2);
void video_restore_cursor(void);

#endif // VIDEO_H

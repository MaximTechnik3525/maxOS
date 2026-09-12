#include "video.h"
#include "../kernel/debug.h"
#include "../kernel/string.h"

video_driver_t g_video;

// Mouse cursor state storage
static int cursor_saved_x = 0;
static int cursor_saved_y = 0;
static int cursor_bg_saved = 0;
static unsigned short cursor_bg[12][12];



// Packed multiboot_info subset for reading framebuffer fields
#pragma pack(push, 1)
struct mb_fb_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;
    unsigned long long framebuffer_addr;
    unsigned int framebuffer_pitch;
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;
    unsigned char framebuffer_bpp;
    unsigned char framebuffer_type;
    unsigned char framebuffer_color_info[6];
};
#pragma pack(pop)

void video_init(void* multiboot_info_ptr) {
    struct mb_fb_info* mbi = (struct mb_fb_info*)multiboot_info_ptr;

    g_video.fb = (unsigned char*)(unsigned long)mbi->framebuffer_addr;
    g_video.width = mbi->framebuffer_width ? mbi->framebuffer_width : 1024;
    g_video.height = mbi->framebuffer_height ? mbi->framebuffer_height : 768;
    g_video.bpp = mbi->framebuffer_bpp ? mbi->framebuffer_bpp : 16;
    g_video.pitch = mbi->framebuffer_pitch ? mbi->framebuffer_pitch : (g_video.width * ((g_video.bpp + 7) / 8));

    g_video.red_pos    = mbi->framebuffer_color_info[0];
    g_video.red_size   = mbi->framebuffer_color_info[1];
    g_video.green_pos  = mbi->framebuffer_color_info[2];
    g_video.green_size = mbi->framebuffer_color_info[3];
    g_video.blue_pos   = mbi->framebuffer_color_info[4];
    g_video.blue_size  = mbi->framebuffer_color_info[5];

    // Detect firmware mode: Bit 1 (boot_device) is set by BIOS GRUB, but not by EFI GRUB
    int is_uefi = ((mbi->flags & (1 << 1)) == 0);
    g_video.is_uefi = is_uefi;

    // Detect format and firmware mode
    if (g_video.bpp == 16) {
        g_video.format = VIDEO_FORMAT_RGB565;
        g_video.mode_name = is_uefi ? "UEFI GOP (16-bpp RGB565)" : "BIOS VBE (16-bpp RGB565)";
    } else if (g_video.bpp == 32) {
        if (g_video.red_pos == 0 && g_video.blue_pos >= 16) {
            g_video.format = VIDEO_FORMAT_RGBA8888;
            g_video.mode_name = is_uefi ? "UEFI GOP (32-bpp RGBA)" : "BIOS VBE (32-bpp RGBA)";
        } else {
            g_video.format = VIDEO_FORMAT_BGRA8888;
            g_video.mode_name = is_uefi ? "UEFI GOP (32-bpp BGRA)" : "BIOS VBE (32-bpp BGRA)";
        }
    } else if (g_video.bpp == 24) {
        g_video.format = (g_video.red_pos == 0) ? VIDEO_FORMAT_RGB888 : VIDEO_FORMAT_BGR888;
        g_video.mode_name = is_uefi ? "UEFI GOP (24-bpp RGB)" : "BIOS VBE (24-bpp RGB)";
    } else {
        g_video.format = VIDEO_FORMAT_UNKNOWN;
        g_video.mode_name = is_uefi ? "UEFI GOP (Unknown BPP)" : "BIOS VBE (Unknown BPP)";
    }

    debug_log("VIDEO", "Unified Video Driver Initialized");
    debug_puts("  Mode: "); debug_puts(g_video.mode_name); debug_puts("\n");
    debug_puts("  Resolution: "); debug_print_num(g_video.width, 10);
    debug_puts("x"); debug_print_num(g_video.height, 10);
    debug_puts("@"); debug_print_num(g_video.bpp, 10); debug_puts("bpp\n");
    debug_puts("  Pitch: "); debug_print_num(g_video.pitch, 10); debug_puts(" bytes/line\n");
    debug_puts("  FB Physical Addr: 0x"); debug_print_num((unsigned long)g_video.fb, 16); debug_puts("\n");
}

int video_is_uefi(void) { return g_video.is_uefi; }
int video_get_width(void) { return g_video.width; }
int video_get_height(void) { return g_video.height; }
int video_get_bpp(void) { return g_video.bpp; }
int video_get_pitch(void) { return g_video.pitch; }
const char* video_get_mode_name(void) { return g_video.mode_name; }

unsigned int video_rgb565_to_native(unsigned short c565) {
    if (g_video.bpp == 16) {
        return c565;
    }
    // Expand 5:6:5 to 8-bit components
    unsigned int r5 = (c565 >> 11) & 0x1F;
    unsigned int g6 = (c565 >> 5)  & 0x3F;
    unsigned int b5 =  c565        & 0x1F;

    unsigned int r8 = (r5 * 527 + 23) >> 6;
    unsigned int g8 = (g6 * 259 + 33) >> 6;
    unsigned int b8 = (b5 * 527 + 23) >> 6;

    if (g_video.format == VIDEO_FORMAT_RGBA8888 || g_video.red_pos == 0) {
        return r8 | (g8 << 8) | (b8 << 16) | 0xFF000000;
    } else {
        // BGRA / BGRX standard for UEFI GOP
        return b8 | (g8 << 8) | (r8 << 16) | 0xFF000000;
    }
}

unsigned short video_native_to_rgb565(unsigned int nat) {
    if (g_video.bpp == 16) {
        return (unsigned short)nat;
    }
    unsigned int r8, g8, b8;
    if (g_video.format == VIDEO_FORMAT_RGBA8888 || g_video.red_pos == 0) {
        r8 = nat & 0xFF;
        g8 = (nat >> 8) & 0xFF;
        b8 = (nat >> 16) & 0xFF;
    } else {
        b8 = nat & 0xFF;
        g8 = (nat >> 8) & 0xFF;
        r8 = (nat >> 16) & 0xFF;
    }
    unsigned short r5 = (r8 * 31 + 127) / 255;
    unsigned short g6 = (g8 * 63 + 127) / 255;
    unsigned short b5 = (b8 * 31 + 127) / 255;
    return (r5 << 11) | (g6 << 5) | b5;
}

void video_put_pixel(int x, int y, unsigned short rgb565) {
    if ((unsigned int)x >= g_video.width || (unsigned int)y >= g_video.height) return;
    unsigned char* row = g_video.fb + y * g_video.pitch;
    if (g_video.bpp == 16) {
        ((unsigned short*)row)[x] = rgb565;
    } else if (g_video.bpp == 32) {
        ((unsigned int*)row)[x] = video_rgb565_to_native(rgb565);
    } else if (g_video.bpp == 24) {
        unsigned int nat = video_rgb565_to_native(rgb565);
        int offset = x * 3;
        row[offset + 0] = nat & 0xFF;
        row[offset + 1] = (nat >> 8) & 0xFF;
        row[offset + 2] = (nat >> 16) & 0xFF;
    }
}

unsigned short video_get_pixel(int x, int y) {
    if ((unsigned int)x >= g_video.width || (unsigned int)y >= g_video.height) return 0;
    unsigned char* row = g_video.fb + y * g_video.pitch;
    if (g_video.bpp == 16) {
        return ((unsigned short*)row)[x];
    } else if (g_video.bpp == 32) {
        return video_native_to_rgb565(((unsigned int*)row)[x]);
    } else if (g_video.bpp == 24) {
        int offset = x * 3;
        unsigned int nat = row[offset + 0] | (row[offset + 1] << 8) | (row[offset + 2] << 16);
        return video_native_to_rgb565(nat);
    }
    return 0;
}

void video_draw_rect(int x, int y, int w, int h, unsigned short rgb565) {
    if (w <= 0 || h <= 0) return;
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + w > (int)g_video.width) ? (int)g_video.width : (x + w);
    int y1 = (y + h > (int)g_video.height) ? (int)g_video.height : (y + h);
    if (x0 >= x1 || y0 >= y1) return;

    int count = x1 - x0;
    if (g_video.bpp == 16) {
        for (int cur_y = y0; cur_y < y1; cur_y++) {
            unsigned short* dst = (unsigned short*)(g_video.fb + cur_y * g_video.pitch) + x0;
            for (int i = 0; i < count; i++) dst[i] = rgb565;
        }
    } else if (g_video.bpp == 32) {
        unsigned int native_color = video_rgb565_to_native(rgb565);
        for (int cur_y = y0; cur_y < y1; cur_y++) {
            unsigned int* dst = (unsigned int*)(g_video.fb + cur_y * g_video.pitch) + x0;
            for (int i = 0; i < count; i++) dst[i] = native_color;
        }
    } else if (g_video.bpp == 24) {
        for (int cur_y = y0; cur_y < y1; cur_y++) {
            for (int cur_x = x0; cur_x < x1; cur_x++) {
                video_put_pixel(cur_x, cur_y, rgb565);
            }
        }
    }
}

void video_draw_char(char c, int start_x, int start_y, unsigned short color) {
    if (c < 32 || c > 126) return;
    int font_idx = c - 32;
    const unsigned char* bitmap = (const unsigned char*)max_font + (font_idx * 8);

    for (int y = 0; y < 8; y++) {
        unsigned char row = bitmap[y];
        for (int x = 0; x < 8; x++) {
            if ((row & (0x80 >> x)) != 0) {
                int px = start_x + x;
                int py = start_y + y;
                if ((unsigned int)px < g_video.width && (unsigned int)py < g_video.height) {
                    // If mouse cursor is over this pixel, update cursor saved bg
                    if (cursor_bg_saved &&
                        px >= cursor_saved_x && px < cursor_saved_x + 12 &&
                        py >= cursor_saved_y && py < cursor_saved_y + 12) {
                        cursor_bg[py - cursor_saved_y][px - cursor_saved_x] = color;
                        if (mouse_arrow[py - cursor_saved_y][px - cursor_saved_x] != 0) {
                            continue;
                        }
                    }
                    video_put_pixel(px, py, color);
                }
            }
        }
    }
}

void video_print_string(const char* str, int x, int y, unsigned short color) {
    if (!str) return;
    int cur_x = x;
    int cur_y = y;
    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            cur_y += 10;
        } else {
            video_draw_char(*str, cur_x, cur_y, color);
            cur_x += 8;
        }
        str++;
    }
}

void video_clear(unsigned short rgb565) {
    video_draw_rect(0, 0, g_video.width, g_video.height, rgb565);
}

void video_scroll(int x, int y, int w, int h, int lines, unsigned short fill_color) {
    if (lines <= 0 || h <= 0) return;
    if (lines >= h) {
        video_draw_rect(x, y, w, h, fill_color);
        return;
    }

    int bytes_per_pixel = (g_video.bpp + 7) / 8;
    int copy_bytes = w * bytes_per_pixel;
    int move_lines = h - lines;

    for (int row = y; row < y + move_lines; row++) {
        unsigned char* dst = g_video.fb + row * g_video.pitch + x * bytes_per_pixel;
        unsigned char* src = g_video.fb + (row + lines) * g_video.pitch + x * bytes_per_pixel;
        memcpy(dst, src, copy_bytes);
    }
    // Clear bottom lines
    video_draw_rect(x, y + move_lines, w, lines, fill_color);
}

void video_render_wallpaper(unsigned short c1, unsigned short c2) {
    // Elegant checkerboard pattern across entire display
    for (int y = 0; y < (int)g_video.height; y++) {
        for (int x = 0; x < (int)g_video.width; x += 16) {
            unsigned short color = (((x ^ y) & 16) == 0) ? c1 : c2;
            int rw = (x + 16 > (int)g_video.width) ? (int)g_video.width - x : 16;
            video_draw_rect(x, y, rw, 1, color);
        }
    }
}

void video_restore_cursor(void) {
    if (!cursor_bg_saved) return;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int sx = cursor_saved_x + x;
            int sy = cursor_saved_y + y;
            if ((unsigned int)sx < g_video.width && (unsigned int)sy < g_video.height) {
                video_put_pixel(sx, sy, cursor_bg[y][x]);
            }
        }
    }
    cursor_bg_saved = 0;
}

void video_draw_cursor(int mouse_x, int mouse_y, unsigned short color1, unsigned short color2) {
    if (cursor_bg_saved) {
        video_restore_cursor();
    }
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x > (int)g_video.width - 12) mouse_x = g_video.width - 12;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y > (int)g_video.height - 12) mouse_y = g_video.height - 12;

    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int sx = mouse_x + x;
            int sy = mouse_y + y;
            if ((unsigned int)sx < g_video.width && (unsigned int)sy < g_video.height) {
                cursor_bg[y][x] = video_get_pixel(sx, sy);
            } else {
                cursor_bg[y][x] = 0x0000;
            }
        }
    }
    cursor_saved_x = mouse_x;
    cursor_saved_y = mouse_y;
    cursor_bg_saved = 1;

    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int sx = mouse_x + x;
            int sy = mouse_y + y;
            if ((unsigned int)sx < g_video.width && (unsigned int)sy < g_video.height) {
                unsigned char pt = mouse_arrow[y][x];
                if (pt == 1) {
                    video_put_pixel(sx, sy, color1);
                } else if (pt == 2) {
                    video_put_pixel(sx, sy, color2);
                } else if (pt == 3) {
                    video_put_pixel(sx, sy, 0x9CD3);
                }
            }
        }
    }
}

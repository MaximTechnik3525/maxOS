// Упакованная структура, чтобы компилятор не добавлял лишних скрытых байт (alignment padding)
#pragma pack(push, 1)
struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];       // Поля для ELF секций
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

    // Графический фреймбуфер (начиная с 88-го байта структуры)
    unsigned long long framebuffer_addr; 
    unsigned int framebuffer_pitch;     // Длина строки в БАЙТАХ
    unsigned int framebuffer_width;     // Ширина экрана в пикселях
    unsigned int framebuffer_height;    // Высота экрана в пикселях
    unsigned char framebuffer_bpp;       // Количество бит на пиксель (16, 24 или 32)
    unsigned char framebuffer_type;
    unsigned char framebuffer_color_info[6];
};
#pragma pack(pop)

#include "maxfs.h"
#include "notepad.h"
#include "installer.h"
#include "explorer.h"
#include "ata.h"
#include "maxp.h"
#include "taskbar.h"
#include "calc.h"
#include "sysinfo.h"
#include "pong.h"
void print_string(char* str, int x, int y, unsigned short color);
void draw_char(char c, int start_x, int start_y, unsigned short color);
unsigned char inb(unsigned short port);
void int_str(int num, char* str);
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short val);
void shutdown();
char scan_code_to_ascii(unsigned char scan_code);
unsigned short* _gfx_memory_backend;
unsigned int REAL_PITCH = 1024;
#define gfx_memory_safe(y, x) _gfx_memory_backend[(y) * REAL_PITCH + (x)]
#define gfx_memory _gfx_memory_backend
void draw_cursor(int mouse_x, int mouse_y);
void draw_btn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_expbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_pongbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_offbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_instbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void get_cpu(char* buffer);
void draw_window();
void wait_mouse(unsigned char type);
void init_mouse();
void prev_cursor();
void play_sound(unsigned int nfreq);
void no_sound();
void pong();
void help();
void cpu_win();
void filew();
void error(char* err);
unsigned short bg_col = 0x18C3;
void sleep(unsigned int ms);
int str_in(char* main_string, char* substring);
unsigned char mouse_arrow[12][12] = {
    {1,1,3,0,0,0,0,0,0,0,0,0},
    {1,2,1,3,0,0,0,0,0,0,0,0},
    {1,2,2,1,3,0,0,0,0,0,0,0},
    {1,2,2,2,1,3,0,0,0,0,0,0},
    {1,2,2,2,2,1,3,0,0,0,0,0},
    {1,2,2,2,2,2,1,3,0,0,0,0},
    {1,2,2,2,2,2,2,1,3,0,0,0},
    {1,2,2,2,2,1,1,1,1,3,0,0},
    {1,2,1,1,2,1,3,3,3,3,0,0},
    {1,1,3,3,1,2,1,3,0,0,0,0},
    {0,0,3,0,0,1,1,1,3,0,0,0},
    {0,0,0,0,0,0,3,3,3,0,0,0}
};
const unsigned char max_font[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 32 (пробел)
    0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00, // 33 !
    0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00, // 34 "
    0x24,0x24,0x7E,0x24,0x7E,0x24,0x24,0x00, // 35 #
    0x08,0x3E,0x68,0x3C,0x16,0x7C,0x10,0x00, // 36 $
    0x63,0x64,0x08,0x10,0x20,0x4C,0x66,0x00, // 37 %
    0x3C,0x66,0x3C,0x38,0x67,0x66,0x3F,0x00, // 38 &
    0x06,0x0C,0x18,0x00,0x00,0x00,0x00,0x00, // 39 '
    0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00, // 40 (
    0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00, // 41 )
    0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00, // 42 *
    0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00, // 43 +
    0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30, // 44 ,
    0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00, // 45 -
    0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00, // 46 .
    0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00, // 47 /
    0x3E,0x61,0x65,0x69,0x6D,0x43,0x3E,0x00, // 48 0
    0x0C,0x1C,0x0C,0x0C,0x0C,0x0C,0x3E,0x00, // 49 1
    0x3E,0x63,0x06,0x1C,0x30,0x60,0x7F,0x00, // 50 2
    0x7F,0x06,0x0C,0x1C,0x06,0x63,0x3E,0x00, // 51 3
    0x1C,0x3C,0x6C,0x6C,0x7F,0x0C,0x1E,0x00, // 52 4
    0x7F,0x60,0x7E,0x03,0x03,0x63,0x3E,0x00, // 53 5
    0x1E,0x30,0x60,0x7E,0x63,0x63,0x3E,0x00, // 54 6
    0x7F,0x43,0x06,0x0C,0x18,0x18,0x18,0x00, // 55 7
    0x3E,0x63,0x63,0x3E,0x63,0x63,0x3E,0x00, // 56 8
    0x3E,0x63,0x63,0x7F,0x03,0x06,0x3C,0x00, // 57 9
    0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00, // 58 :
    0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00, // 59 ;
    0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00, // 60 <
    0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00, // 61 =
    0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00, // 62 >
    0x3E,0x63,0x06,0x0C,0x18,0x00,0x18,0x00, // 63 ?
    0x3E,0x63,0x6F,0x6B,0x6B,0x60,0x3E,0x00, // 64 @
    0x18,0x3C,0x66,0x66,0x7F,0x66,0x66,0x00, // 65 A
    0x7E,0x63,0x63,0x7C,0x63,0x63,0x7E,0x00, // 66 B
    0x3E,0x63,0x60,0x60,0x60,0x63,0x3E,0x00, // 67 C
    0x7C,0x66,0x63,0x63,0x63,0x66,0x7C,0x00, // 68 D
    0x7F,0x60,0x60,0x7C,0x60,0x60,0x7F,0x00, // 69 E
    0x7F,0x60,0x60,0x7C,0x60,0x60,0x60,0x00, // 70 F
    0x3E,0x63,0x60,0x6F,0x63,0x63,0x3E,0x00, // 71 G
    0x66,0x66,0x66,0x7F,0x66,0x66,0x66,0x00, // 72 H
    0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00, // 73 I
    0x1F,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00, // 74 J
    0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00, // 75 K
    0x60,0x60,0x60,0x60,0x60,0x60,0x7F,0x00, // 76 L
    0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00, // 77 M
    0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00, // 78 N
    0x3E,0x63,0x63,0x63,0x63,0x63,0x3E,0x00, // 79 O
    0x7E,0x63,0x63,0x7E,0x60,0x60,0x60,0x00, // 80 P
    0x3E,0x63,0x63,0x63,0x6B,0x66,0x3D,0x00, // 81 Q
    0x7E,0x63,0x63,0x7E,0x70,0x6C,0x66,0x00, // 82 R
    0x3E,0x63,0x60,0x3E,0x03,0x63,0x3E,0x00, // 83 S
    0x7F,0x18,0x18,0x18,0x18,0x18,0x18,0x00, // 84 T
    0x66,0x66,0x66,0x66,0x66,0x66,0x3E,0x00, // 85 U
    0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00, // 86 V
    0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00, // 87 W
    0x63,0x63,0x34,0x1C,0x34,0x63,0x63,0x00, // 88 X
    0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00, // 89 Y
    0x7F,0x03,0x06,0x0C,0x18,0x30,0x7F,0x00, // 90 Z
    0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00, // 91 [
    0x00,0x40,0x20,0x10,0x08,0x04,0x02,0x00, // 92 backslash
    0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00, // 93 ]
    0x14,0x22,0x00,0x00,0x00,0x00,0x00,0x00, // 94 ^
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF, // 95 _
    0x18,0x18,0x0C,0x00,0x00,0x00,0x00,0x00, // 96 `
    0x00,0x00,0x3E,0x03,0x3F,0x63,0x3D,0x00, // 97 a
    0x60,0x60,0x7C,0x66,0x63,0x66,0x7C,0x00, // 98 b
    0x00,0x00,0x3E,0x60,0x60,0x63,0x3E,0x00, // 99 c
    0x03,0x03,0x3F,0x63,0x63,0x63,0x3D,0x00, // 100 d
    0x00,0x00,0x3E,0x63,0x7F,0x60,0x3E,0x00, // 101 e
    0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00, // 102 f
    0x00,0x00,0x3D,0x63,0x63,0x3F,0x03,0x3E, // 103 g
    0x60,0x60,0x7C,0x66,0x63,0x66,0x66,0x00, // 104 h
    0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00, // 105 i
    0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x3C, // 106 j
    0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00, // 107 k
    0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00, // 108 l
    0x00,0x00,0x66,0x7F,0x6B,0x63,0x63,0x00, // 109 m
    0x00,0x00,0x7C,0x66,0x63,0x66,0x66,0x00, // 110 n
    0x00,0x00,0x3E,0x63,0x63,0x63,0x3E,0x00, // 111 o
    0x00,0x00,0x7C,0x66,0x63,0x7C,0x60,0x60, // 112 p
    0x00,0x00,0x3D,0x63,0x63,0x3F,0x03,0x03, // 113 q
    0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00, // 114 r
    0x00,0x00,0x3E,0x60,0x3E,0x03,0x3E,0x00, // 115 s
    0x30,0x30,0x7C,0x30,0x30,0x34,0x18,0x00, // 116 t
    0x00,0x00,0x63,0x63,0x63,0x66,0x3D,0x00, // 117 u
    0x00,0x00,0x63,0x63,0x63,0x3C,0x18,0x00, // 118 v
    0x00,0x00,0x63,0x63,0x6B,0x7F,0x36,0x00, // 119 w
    0x00,0x00,0x63,0x34,0x1C,0x34,0x63,0x00, // 120 x
    0x00,0x00,0x63,0x63,0x63,0x3F,0x03,0x3E, // 121 y
    0x00,0x00,0x7F,0x0C,0x18,0x30,0x7F,0x00, // 122 z
    0x0E,0x18,0x18,0x30,0x18,0x18,0x0E,0x00, // 123 {
    0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00, // 124 |
    0x70,0x18,0x18,0x0C,0x18,0x18,0x70,0x00, // 125 }
    0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00  // 126 ~
};
int win_x = 150;
int win_y = 140;
int win_w = 740;
int win_h = 550;
int pos_x = 512;
int pos_y = 384;
unsigned short cursor_bg[12][12];
int cursor_bg_saved = 0;
int cursor_saved_x = -1;
int cursor_saved_y = -1;
int theme = 5;
int w_mode = 0;
int km_mode = 0;
// PONG
int pad_x = 0;
int pad_y = 0;
int pad_w = 60, pad_h = 20;
int ball_x = 500, ball_y = 360;
int ball_dx = 3;
int ball_dy = 3;
int ball_size = 8;
int game = 0;
int collisions = 0;
int drag = 2;
int fid = 0;
int tail = 0;
int repeats = 1;
int corners = 0;
void kmain(unsigned long multiboot_info_address, unsigned long magic) {
    (void)magic;
    struct multiboot_info* mbi = (struct multiboot_info*) multiboot_info_address;
    _gfx_memory_backend = (unsigned short*)(unsigned long)mbi->framebuffer_addr;
    if (mbi->framebuffer_pitch > 0) { REAL_PITCH = mbi->framebuffer_pitch / 2; }

    init_mouse();

    // Splash screen
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 387 && y >= 384) {
                gfx_memory[y * 1024 + x] = 0x0DE5;
            } else if (y <= 393 && y > 387) {
                gfx_memory[y * 1024 + x] = 0x03EA;
            } else if (y <= 400 && y > 393) {
                gfx_memory[y * 1024 + x] = 0x01A4;
            } else {
                gfx_memory[y * 1024 + x] = 0x0000;
            }
        }
    }
    print_string("maxOS RedCycle x86_64", 400, 420, 0x0DE5);
    print_string("by maxTech", 10, 10, 0x24EE);
    play_sound(100); sleep(150); play_sound(200); sleep(150); play_sound(400); sleep(150); play_sound(600); sleep(150); play_sound(50); sleep(200); no_sound();
    sleep(1500);

    // Initialize all filesystem, application, and GUI subsystems
    maxfs_init();
    maxp_init();
    taskbar_init();
    notepad_init();
    installer_init();
    explorer_init();
    calc_init();
    sysinfo_init();
    pong_init();

    drag = 0;
    draw_window();

    unsigned char packet[3];
    int clock_timer = 0;

    while (1) {
        unsigned char status = inb(0x64);
        if (status & 0x01) {
            if ((status & 0x20) && w_mode == 0 && drag != 2) {
                // PS/2 Mouse packet
                packet[0] = inb(0x60);
                if ((packet[0] & 0x08) == 0) { continue; }
                int timeout = 100000;
                while (!(inb(0x64) & 0x01) && timeout--);
                packet[1] = inb(0x60);
                timeout = 100000;
                while (!(inb(0x64) & 0x01) && timeout--);
                packet[2] = inb(0x60);

                int sign_x = packet[0] & 0x10;
                int sign_y = packet[0] & 0x20;
                int click = packet[0] & 0x01;
                int delta_x = packet[1];
                int delta_y = packet[2];
                if (sign_x) delta_x |= 0xFFFFFF00;
                if (sign_y) delta_y |= 0xFFFFFF00;

                if ((delta_x > -100 && delta_x < 100) && (delta_y > -100 && delta_y < 100)) {
                    if (delta_x != 0 || delta_y != 0) {
                        if (tail == 0) { prev_cursor(); }
                        pos_x += delta_x / 3;
                        pos_y -= delta_y / 3;
                        if (pos_x > 1024 - 12) pos_x = 1024 - 12;
                        if (pos_x < 0) pos_x = 0;
                        if (pos_y > 768 - 12) pos_y = 768 - 12;
                        if (pos_y < 0) pos_y = 0;
                        draw_cursor(pos_x, pos_y);
                    }

                    if (click == 1) {
                        if (taskbar_handle_click(pos_x, pos_y)) {
                            continue;
                        }
                        if (notepad_open) {
                            if (notepad_handle_click(pos_x, pos_y)) continue;
                        }
                        if (explorer_open) {
                            if (explorer_handle_click(pos_x, pos_y)) continue;
                        }
                        if (calc_open) {
                            if (calc_handle_click(pos_x, pos_y)) continue;
                        }
                        if (sysinfo_open) {
                            if (sysinfo_handle_click(pos_x, pos_y)) continue;
                        }
                        if (pong_open) {
                            if (pong_handle_click(pos_x, pos_y)) continue;
                        }
                        if (installer_open) {
                            if (installer_handle_click(pos_x, pos_y)) continue;
                        }
                    }
                }
            } else {
                // PS/2 Keyboard scancode
                unsigned char scan_code = inb(0x60);
                if (scan_code < 0x80 && drag != 2) {
                    char ascii_char = scan_code_to_ascii(scan_code);

                    if (taskbar_handle_key(ascii_char, scan_code)) {
                        continue;
                    }
                    if (notepad_open) {
                        if (notepad_handle_key(ascii_char, scan_code)) continue;
                    }
                    if (explorer_open) {
                        if (explorer_handle_key(ascii_char, scan_code)) continue;
                    }
                    if (calc_open) {
                        if (calc_handle_key(ascii_char, scan_code)) continue;
                    }
                    if (sysinfo_open) {
                        if (sysinfo_handle_key(ascii_char, scan_code)) continue;
                    }
                    if (pong_open) {
                        if (pong_handle_key(ascii_char, scan_code)) continue;
                    }
                    if (installer_open) {
                        if (installer_handle_key(ascii_char, scan_code)) continue;
                    }

                    // Start Menu shortcut (Win key / M)
                    if (ascii_char == 'm' || ascii_char == 'M' || scan_code == 0x5B || scan_code == 0x5C) {
                        taskbar_toggle_start_menu();
                        continue;
                    }

                    // Close active window
                    if (ascii_char == 'c' || ascii_char == 'C' || scan_code == 0x01) {
                        maxp_close_all_windows();
                        draw_window();
                        continue;
                    }

                    // Quick app launchers
                    if (ascii_char == 'F' || ascii_char == 'f') { maxp_launch_app(MAXP_APP_NOTEPAD); continue; }
                    if (ascii_char == 'E' || ascii_char == 'e') { maxp_launch_app(MAXP_APP_EXPLORER); continue; }
                    if (ascii_char == 'K' || ascii_char == 'k') { maxp_launch_app(MAXP_APP_CALC); continue; }
                    if (ascii_char == 'S' || ascii_char == 's') { maxp_launch_app(MAXP_APP_SYSINFO); continue; }
                    if (ascii_char == 'P' || ascii_char == 'p') { maxp_launch_app(MAXP_APP_PONG); continue; }
                    if (ascii_char == 'I' || ascii_char == 'i') { maxp_launch_app(MAXP_APP_INSTALLER); continue; }

                    // Themes 1-9
                    if (ascii_char >= '1' && ascii_char <= '9') {
                        theme = ascii_char - '0';
                        if (theme == 1) bg_col = 0x18C3;
                        else if (theme == 2) bg_col = 0x2000;
                        else if (theme == 3) bg_col = 0x1041;
                        else if (theme == 4) bg_col = 0x10A2;
                        else if (theme == 5) bg_col = 0x01C8;
                        else if (theme == 6) bg_col = 0x00A1;
                        else if (theme == 7) bg_col = 0x4083;
                        else if (theme == 8) bg_col = 0x7BE0;
                        else if (theme == 9) bg_col = 0x0110;
                        draw_window();
                        play_sound(700); sleep(30); no_sound();
                        continue;
                    }

                    // Rounded corners toggle
                    if (ascii_char == 'M') { corners = 1; draw_window(); play_sound(700); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'N') { corners = 0; draw_window(); play_sound(500); sleep(50); no_sound(); continue; }

                    // Mouse trail toggle
                    if (ascii_char == 'C' && tail == 0) { tail = 1; play_sound(900); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'O' && tail == 1) { tail = 0; draw_window(); play_sound(800); sleep(50); no_sound(); continue; }

                    // Key-mouse mode
                    if (ascii_char == 'T') { km_mode = 1; play_sound(200); sleep(50); no_sound(); continue; }
                    if (ascii_char == 'G') { km_mode = 0; play_sound(1000); sleep(50); no_sound(); draw_window(); continue; }

                    if (km_mode == 1) {
                        if (ascii_char == 'U' && pos_y >= 15) { if (tail == 0) prev_cursor(); pos_y -= 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'D' && pos_y <= 768 - 27) { if (tail == 0) prev_cursor(); pos_y += 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'R' && pos_x <= 1024 - 27) { if (tail == 0) prev_cursor(); pos_x += 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'L' && pos_x >= 15) { if (tail == 0) prev_cursor(); pos_x -= 15; draw_cursor(pos_x, pos_y); }
                        if (ascii_char == 'e') {
                            if (taskbar_handle_click(pos_x, pos_y)) continue;
                            if (notepad_open && notepad_handle_click(pos_x, pos_y)) continue;
                            if (explorer_open && explorer_handle_click(pos_x, pos_y)) continue;
                            if (calc_open && calc_handle_click(pos_x, pos_y)) continue;
                            if (sysinfo_open && sysinfo_handle_click(pos_x, pos_y)) continue;
                            if (pong_open && pong_handle_click(pos_x, pos_y)) continue;
                            if (installer_open && installer_handle_click(pos_x, pos_y)) continue;
                        }
                    }

                    // Format disk
                    if (ascii_char == 'f' && drag == 0) {
                        maxfs_format("maxOS Disk");
                        play_sound(800); sleep(100); no_sound();
                    }
                }
            }
        } else {
            if (pong_open) {
                pong_tick();
                sleep(16);
            } else {
                clock_timer++;
                if (clock_timer >= 100000) {
                    clock_timer = 0;
                    taskbar_draw_clock();
                }
            }
        }
    }
}

int score = 0;
void pong(void) {
    maxp_launch_app(MAXP_APP_PONG);
}

void filew(void) {
    maxp_launch_app(MAXP_APP_EXPLORER);
}

void help(void) {
    notepad_open_file_by_name("readme.txt");
    maxp_set_active_app(MAXP_APP_NOTEPAD);
}

void cpu_win(void) {
    maxp_launch_app(MAXP_APP_SYSINFO);
}

void int_str(int num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    str[i] = '\0';
    int len = i;
    for (int j = 0; j < len / 2; j++) {
        char temp = str[j];
        str[j] = str[len - 1 - j];
        str[len - 1 - j] = temp;
    }
}
void sleep(unsigned int ms) {
    for (unsigned int i = 0; i < ms; i++) {
        outb(0x43, 0x30);
        outb(0x40, 0xA9);
        outb(0x40, 0x04);
        unsigned char status = 0;
        while ((status & 0x80) == 0) {
            outb(0x43, 0xE2);
            status = inb(0x40);
        }
    }
}
void outw(unsigned short port, unsigned short val) { __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port)); }
void shutdown() {
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 387 && y >= 384) {
                gfx_memory[y * 1024 + x] = 0xB269;
            }
            else if (y <= 393 && y > 387) {
                gfx_memory[y * 1024 + x] = 0x81C6;
            }
            else if (y <= 400 && y > 393) {
                gfx_memory[y * 1024 + x] = 0x4924;
            }
            else { gfx_memory[y * 1024 + x] = 0x0000; }

        }
    }
    print_string("maxOS is shutting down...", 420, 420, 0xB269);
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(150); no_sound();
    play_sound(50); sleep(250); no_sound();
    sleep(3000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x0B004, 0x2000);
    error("Cannot use power off ports! Code: 0x01");
    asm volatile("cli; hlt");
}
void error(char* err) {
    drag = 2;
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            gfx_memory[y * 1024 + x] = 0xB269;
        }
    }
    print_string("maxOS error!", 470, 10, 0xFFFF);
    print_string(err, 10, 30, 0xFFFF);
    print_string("Restart or off your PC :(", 10, 50, 0xFFFF);
    play_sound(100); sleep(250); play_sound(75); sleep(250); play_sound(50); sleep(250); no_sound();
}
int str_in(char* main_string, char* substring) {
    int i = 0;
    if (substring[0] == '\0') return 1;
    while (main_string[i] != '\0') {
        int j = 0;
        while (main_string[i + j] == substring[j] && substring[j] != '\0') { j++; }
        if (substring[j] == '\0') { return 1; }
        i++;
    }
    return 0;
}
void play_sound(unsigned int nfreq) {
    unsigned int div;
    unsigned char tmp;
    div = 1193180 / nfreq;
    outb(0x43, 0xB6);
    outb(0x42, (unsigned char) (div & 0xFF));
    outb(0x42, (unsigned char) ((div >> 8) & 0xFF));
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}
void no_sound() {
    unsigned char tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}
void prev_cursor() {
    if (!cursor_bg_saved) return;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (mouse_arrow[y][x] != 0) {
                int cur_x = cursor_saved_x + x;
                int cur_y = cursor_saved_y + y;
                if (cur_x >= 0 && cur_x < 1024 && cur_y >= 0 && cur_y < 768) {
                    gfx_memory[cur_y * 1024 + cur_x] = cursor_bg[y][x];
                }
            }
        }
    }
    cursor_bg_saved = 0;
}
void outb(unsigned short port, unsigned char data) {__asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));}
void wait_mouse(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
    else {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    }
}
void init_mouse() {
    wait_mouse(0);
    outb(0x64, 0xA8);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0xF4);
    wait_mouse(1);
    inb(0x60);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0xE8);
    wait_mouse(1);
    inb(0x60);
    wait_mouse(0);
    outb(0x64, 0xD4);
    wait_mouse(0);
    outb(0x60, 0x00);
    wait_mouse(1);
    inb(0x60);
}

void get_cpu(char* buffer) {
    unsigned int registers[4];
    for (unsigned int i = 0; i < 3; i++) {
        unsigned int id = 0x80000002 + i;
        __asm__ __volatile__ ("cpuid" : "=a"(registers[0]), "=b"(registers[1]), "=c"(registers[2]), "=d"(registers[3]) : "a"(id));
    
        for (int reg = 0; reg < 4; reg++) {
            for (int byte = 0; byte < 4; byte++) {
                *buffer = (registers[reg] >> (byte * 8)) & 0xFF;
                buffer++;
            }
        }
    }
    *buffer = 0;
}

void draw_btn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Help", txt_pos_x, txt_pos_y, 0x0000);
}

void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("CPU", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Note", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_expbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Exp", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_pongbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Pong", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_offbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Off", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_instbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 1024 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 1024 + x] = 0xC618;
        }
    }
    print_string("Inst", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_cursor(int mouse_x, int mouse_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x >= 0 && screen_x < 1024 && screen_y >= 0 && screen_y < 768) {
                cursor_bg[y][x] = gfx_memory[screen_y * 1024 + screen_x];
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
            unsigned char pixel_type = mouse_arrow[y][x];
            if (pixel_type == 0) continue;
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x >= 0 && screen_x < 1024 && screen_y >= 0 && screen_y < 768) {
                if (theme == 1) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFFFF;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 2) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xF800;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 3) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4080;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xB269;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 4) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x3186;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xD69F;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 5) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0168;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0x07FF;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 6) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0200;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0x07E0;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 7) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x60A4;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFDF3;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 8) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4962;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xF621;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                else if (theme == 9) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x5000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFBE0;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
            }
        }
    }
}

unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

char scan_code_to_ascii(unsigned char scan_code) {
    switch (scan_code) {
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        case 0x3B: return 'F';
        case 0x3C: return 'S';
        case 0x3D: return 'T';
        case 0x3E: return 'G';
        case 0x3F: return 'C';
        case 0x40: return 'O';
        case 0x41: return 'M';
        case 0x42: return 'N';
        case 0x39: return ' ';
        case 0x2E: return 'c';
        case 0x48: return 'U';
        case 0x50: return 'D';
        case 0x4D: return 'R';
        case 0x4B: return 'L';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x02: return '1';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';
        case 0x20: return 'd';
        case 0x26: return 'l';
        case 0x13: return 'r';
        case 0x16: return 'u';
        case 0x0E: return 'B'; // Backspace
        case 0x1C: return '\n'; // Enter
        case 0x0F: return '\t'; // Tab
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x27: return ';';
        case 0x28: return '\'';
        default: return 0;
    }
}

void draw_char(char c, int start_x, int start_y, unsigned short color) {
    if (c < 32 || c > 126) return;
    int font_idx = c - 32;
    unsigned char* bitmap = (unsigned char*)max_font + (font_idx * 8);
    for (int y = 0; y < 8; y++) {
        unsigned char row = bitmap[y];
        for (int x = 0; x < 8; x++) {
            if ((row & (0x80 >> x)) != 0) {
                gfx_memory[(start_y + y) * 1024 + (start_x + x)] = color;
            }
        }
    }
}
unsigned char bcd_to_binary(unsigned char bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}
unsigned char read_rtc_register(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}
void clock() {
    unsigned char raw_sec = read_rtc_register(0x00);
    unsigned char raw_min = read_rtc_register(0x02);
    unsigned char raw_hour = read_rtc_register(0x04);
    int sec = bcd_to_binary(raw_sec), min = bcd_to_binary(raw_min), hour = bcd_to_binary(raw_hour);
    char s[4], m[4], h[4];
    int_str(sec, s);
    int_str(min, m);
    int_str(hour, h);
    if (theme != 8) {
        print_string(h, win_x + 690, win_y + 6, 0xFFFF);
        print_string(":", win_x + 710, win_y + 6, 0xFFFF);
        print_string(m, win_x + 720, win_y + 6, 0xFFFF);
    }
    if (theme == 8) {
        print_string(h, win_x + 690, win_y + 6, 0x20C0);
        print_string(":", win_x + 710, win_y + 6, 0x20C0);
        print_string(m, win_x + 720, win_y + 6, 0x20C0);
    }
}
void win_corners() {
    gfx_memory[win_y * 1024 + win_x] = bg_col;
    gfx_memory[win_y * 1024 + (win_x+1)] = bg_col;
    gfx_memory[(win_y+1) * 1024 + win_x] = bg_col;
    int right_edge = win_x + win_w - 1;
    gfx_memory[win_y * 1024 + right_edge] = bg_col;
    gfx_memory[win_y * 1024 + (right_edge+1)] = bg_col;
    gfx_memory[(win_y+1) * 1024 + right_edge] = bg_col;
}
void draw_window() {
    cursor_bg_saved = 0;
    // 1. Draw desktop wallpaper pattern for entire desktop
    for (int y = 0; y < 730; y++) {
        int row_offset = y << 10;
        for (int x = 0; x < 1024; x++) {
            if (theme == 1) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x10A2; }
                else { gfx_memory[row_offset + x] = 0x2124; }
            } else if (theme == 2) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x4002; }
                else { gfx_memory[row_offset + x] = 0x8085; }
            } else if (theme == 3) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x2080; }
                else { gfx_memory[row_offset + x] = 0x4100; }
            } else if (theme == 4) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x10A2; }
                else { gfx_memory[row_offset + x] = 0x2945; }
            } else if (theme == 5) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x010A; }
                else { gfx_memory[row_offset + x] = 0x03EF; }
            } else if (theme == 6) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x0102; }
                else { gfx_memory[row_offset + x] = 0x05E0; }
            } else if (theme == 7) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x4004; }
                else { gfx_memory[row_offset + x] = 0xFBEF; }
            } else if (theme == 8) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0xE62F; }
                else { gfx_memory[row_offset + x] = 0x8B04; }
            } else if (theme == 9) {
                if (((x ^ y) & 16) == 0) { gfx_memory[row_offset + x] = 0x0801; }
                else { gfx_memory[row_offset + x] = 0x2000; }
            }
        }
    }

    // 2. Desktop icons on the left side
    taskbar_draw_desktop_icons();

    // 3. Desktop center widget if no app is active or if active app is minimized
    int active_app = maxp_get_active_app();
    if (active_app == MAXP_APP_NONE || taskbar_is_app_minimized()) {
        int card_x = 220, card_y = 150, card_w = 640, card_h = 370;
        for (int y = card_y; y < card_y + card_h; y++) {
            for (int x = card_x; x < card_x + card_w; x++) {
                if (y == card_y || y == card_y + card_h - 1 || x == card_x || x == card_x + card_w - 1) {
                    gfx_memory[y * 1024 + x] = 0x0000;
                } else if (y < card_y + 24) {
                    gfx_memory[y * 1024 + x] = 0x11EB;
                } else {
                    gfx_memory[y * 1024 + x] = 0xEF59;
                }
            }
        }
        print_string("Welcome to maxOS RedCycle x86_64", card_x + 14, card_y + 6, 0xFFFF);
        print_string("Desktop & Applications Environment", card_x + 25, card_y + 36, 0x11EB);
        print_string("Modular applications use .maxP executable files on maxFS 2.0", card_x + 25, card_y + 60, 0x0000);
        print_string("- Launch applications from Desktop icons on the left side", card_x + 25, card_y + 88, 0x0000);
        print_string("- Or click [maxOS] Start Menu button on bottom-left", card_x + 25, card_y + 110, 0x0000);
        print_string("- Active apps appear as tabs in the Taskbar (toggle minimize)", card_x + 25, card_y + 132, 0x0000);
        print_string("Registered .maxP Programs:", card_x + 25, card_y + 162, 0x0200);
        print_string("[Notepad.maxP] [Explorer.maxP] [Calc.maxP]", card_x + 35, card_y + 184, 0x03EA);
        print_string("[SysInfo.maxP] [Pong.maxP]     [Install.maxP]", card_x + 35, card_y + 206, 0x24EE);
        print_string("Hotkeys: Keys 1-9 switch themes | Esc/c closes active app", card_x + 25, card_y + 242, 0x7BEF);
        print_string("System Status: 64-bit Long Mode | PML4 Paging | ATA Ready", card_x + 25, card_y + 270, 0x0000);
        print_string("maxOS Desktop v2.0 - by maxTech", card_x + 25, card_y + 310, 0x8085);
    } else {
        // Render active window
        if (notepad_open) { notepad_draw(); }
        else if (explorer_open) { explorer_draw(); }
        else if (calc_open) { calc_draw(); }
        else if (sysinfo_open) { sysinfo_draw(); }
        else if (pong_open) { pong_draw(); }
        else if (installer_open) { installer_draw(); }
    }

    // 4. Taskbar across the bottom
    taskbar_draw();

    // 5. Cursor
    draw_cursor(pos_x, pos_y);
}

void print_string(char* str, int x, int y, unsigned short color) {
    while (*str != 0) {
        draw_char(*str, x, y, color);
        x += 9;
        str++;
    }
}
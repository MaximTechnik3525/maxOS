struct VirtualFile {
    char name[12];
    int size;
    char content[100];
    int exists;
};
struct VirtualFile ram_disk[5];
void print_string(char* str, int x, int y, unsigned short color);
void draw_char(char c, int start_x, int start_y, unsigned short color);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short val);
void shutdown();
char scan_code_to_ascii(unsigned char scan_code);
unsigned short* gfx_memory;
void draw_cursor(int mouse_x, int mouse_y);
void draw_btn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_expbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_pongbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_offbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
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
unsigned short bg_col = 0x18C3;
void sleep(unsigned int ms);
int str_in(char* main_string, char* substring);
int create_file(char* name, char* text);
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
    0x00,0x40,0x20,0x10,0x08,0x04,0x02,0x00, // 92 \
    |
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



int win_x = 512;
int win_y = 140;
int win_w = 740;
int win_h = 550;
int pos_x = 512;
int pos_y = 384;
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
void kmain() {
    win_x = 150;
    win_y = 140;
    char ftext[100];
    int textid = 0;
    ftext[0] = '\0';
    int fid = 0;
    unsigned char* vesa_data = (unsigned char*) 0x9000;
    unsigned int offset40 = *(unsigned int*)(vesa_data + 40);
    unsigned int offset44 = *(unsigned int*)(vesa_data + 44);
    unsigned int final_address = 0;
    if (offset40 >= 0x00100000) {final_address = offset40;}
    else if (offset44 >= 0x00100000) {final_address = offset44;}
    else {
        for (int i = 32; i < 64; i += 4) {
            unsigned int check_addr = *(unsigned int*)(vesa_data + i);
            if (check_addr >= 0x10000000 && (check_addr & 0x00FFFFFF) == 0) {
                final_address = check_addr;
                break;
            }
        }
    }
    if (final_address == 0) {final_address = 0xFD000000;}
    gfx_memory = (unsigned short*) final_address;
    draw_window();
    draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
    draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
    draw_cursor(pos_x, pos_y);
    int help_col = win_y + 35;
    init_mouse();
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 387 && y >= 384) {
                gfx_memory[y * 1024 + x] = 0x0DE5;
            }
            else if (y <= 393 && y > 387) {
                gfx_memory[y * 1024 + x] = 0x03EA;
            }
            else if (y <= 400 && y > 393) {
                gfx_memory[y * 1024 + x] = 0x01A4;
            }
            else { gfx_memory[y * 1024 + x] = 0x0000; }

        }
    }
    print_string("maxOS ClockWork", 440, 420, 0x0DE5);
    print_string("by maxTech", 10, 10, 0x24EE);
    play_sound(100); sleep(150); play_sound(200); sleep(150); play_sound(400); sleep(150); play_sound(600); sleep(150); play_sound(50); sleep(200); no_sound();
    sleep(2000); draw_window(); drag = 0;
    unsigned char packet[3];
    for (int i = 0; i < 5; i++) {
        ram_disk[i].size = 0;
        ram_disk[i].exists = 0;
        for (int n = 0; n < 12; n++) {
            ram_disk[i].name[n] = '\0';
        }
        for (int t = 0; t < 100; t++) {
            ram_disk[i].content[t] = '\0';
        }
    }
    while(1) {
        unsigned char status = inb(0x64);
        if (status & 0x01) {
            if (status & 0x20 && drag == 0 && w_mode == 0) {
                packet[0] = inb(0x60);
                if ((packet[0] & 0x08) == 0) {continue;}
                int timeout = 100000;
                while (!(inb(0x64) & 0x01) &&  timeout--);
                packet[1] = inb(0x60);
                timeout = 100000;
                while (!(inb(0x64) & 0x01) &&  timeout--);
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
                        prev_cursor();
                        pos_x += delta_x / 3;
                        pos_y -= delta_y / 3;
                        if (pos_x > win_x + 710) {pos_x = win_x + 710;}
                        if (pos_x < win_x + 12) {pos_x = win_x + 12;}
                        if (pos_y > win_y + 538) {pos_y = win_y + 538;}
                        if (pos_y < win_y + 22) {pos_y = win_y + 22;}
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (click == 1) { // MOUSE CLICKS
                        if (pos_x >= win_x + 250 && pos_x <= win_x + 290 && pos_y <= win_y + 30)  { pong(); }
                        if (pos_x >= win_x + 310 && pos_x <= win_x + 350 && pos_y <= win_y + 30)  { shutdown(); }
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30) { help(); }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30) { cpu_win(); }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            create_file("Unnamed.txt", ftext);
                            print_string(ram_disk[fid].name, win_x + 27, win_y + 27, 0xFFFF);
                            print_string(ram_disk[fid].content, win_x + 30, help_col, 0x0000);
                            print_string("C: close.", win_x + 30, help_col + 15, 0x0000);
                            if (fid <= 4) {
                                fid++;
                            }
                            if (str_in(ftext, "theme1")) {
                                theme = 1;
                                draw_window();
                                drag = 0;
                                bg_col = 0x18C3;
                            }
                            if (str_in(ftext, "theme2")) {
                                theme = 2;
                                draw_window();
                                drag = 0;
                                bg_col = 0x2000;
                            }
                            if (str_in(ftext, "theme3")) {
                                theme = 3;
                                draw_window();
                                drag = 0;
                                bg_col = 0x1041;
                            }
                            if (str_in(ftext, "theme4")) {
                                theme = 4;
                                draw_window();
                                drag = 0;
                                bg_col = 0x10A2;
                            }
                            if (str_in(ftext, "theme5")) {
                                theme = 5;
                                draw_window();
                                drag = 0;
                                bg_col = 0x01C8;
                            }
                            if (str_in(ftext, "theme6")) {
                                theme = 6;
                                draw_window();
                                drag = 0;
                                bg_col = 0x00A1;
                            }
                            if (str_in(ftext, "theme7")) {
                                theme = 7;
                                draw_window();
                                drag = 0;
                                bg_col = 0x4083;
                            }
                            if (str_in(ftext, "format")) {
                                textid = 0;
                                for (int i = 0; i < 99; i++) {
                                    ftext[i] = '\0';}
                                for (int i = 0; i < 5; i++) {
                                    ram_disk[i].exists = 0;
                                    ram_disk[i].size = 0;
                                    for (int n = 0; n < 12; n++) {
                                        ram_disk[i].name[n] = '\0';
                                    }
                                    for (int t = 0; t < 100; t++) {
                                        ram_disk[i].content[t] = '\0';
                                    }
                                }
                                fid = 0;
                            }
                            if (str_in(ftext, "clear")) {
                                drag = 0;
                                help_col = 65;
                                draw_window();
                            }
                            if (str_in(ftext, "winrght")) {
                                drag = 0;
                                win_x += 50;
                                draw_window();
                            }
                            if (str_in(ftext, "winlft")) {
                                drag = 0;
                                win_x -= 50;
                                draw_window();
                            }
                            if (str_in(ftext, "winup")) {
                                drag = 0;
                                win_y -= 50;
                                draw_window();
                            }
                            if (str_in(ftext, "windwn")) {
                                drag = 0;
                                win_y += 50;
                                draw_window();
                            }
                            if (str_in(ftext, "spkr")) {
                                play_sound(750);
                                sleep(250);
                                no_sound();
                            }
                            if (str_in(ftext, "scrblck")) {
                                for (int y = 0; y < 768; y++) {
                                    for (int x = 0; x < 1024; x++) {
                                        gfx_memory[y * 1024 + x] = 0x0000;
                                    }
                                }
                            }
                            if (str_in(ftext, "scrwht")) {
                                for (int y = 0; y < 768; y++) {
                                    for (int x = 0; x < 1024; x++) {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            if (str_in(ftext, "stbusy")) {
                                drag = 1;
                            }
                            if (str_in(ftext, "stfree")) { drag = 0; }
                            if (str_in(ftext, "prntstr")) { print_string(ftext, 500, 359, 0x0000); }
                        }
                        if (pos_x >= win_x + 190 && pos_x <= win_x + 230 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            int line = win_y + 65;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else if (y < win_y + 60) {
                                        gfx_memory[y * 1024 + x] = 0xC618;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            print_string("Explorer", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Name:", win_x + 35, win_y + 45, 0x0000);
                            print_string("Size:", win_w - 35, win_y + 45, 0x0000);
                            for (int i = 0; i < 5; i++) {
                                print_string(ram_disk[i].name, win_x + 30, line, 0x0000);
                                char size_str[16];
                                int_str(ram_disk[i].size, size_str);
                                print_string(size_str, win_w - 30, line, 0x0000);
                                line += 15;
                            }
                        }
                    }
                }   
            }
            else {
                unsigned char scan_code = inb(0x60);
                if (scan_code < 0x80 && w_mode == 0 && drag != 2) { // KEYBOARD CLICKS
                    char ascii_char = scan_code_to_ascii(scan_code);
                    if (ascii_char == 'R' && win_x < 300 && drag == 0 && km_mode == 0) {
                        win_x += 20;
                        draw_window();
                    }
                    if (ascii_char == 'L' && win_x > 0 && drag == 0 && km_mode == 0) {
                        win_x-= 20;
                        draw_window();
                    }
                    if (ascii_char == 'D' && win_y < 250 && drag == 0 && km_mode == 0) {
                        win_y += 20;
                        draw_window();
                    }
                    if (ascii_char == 'U' && win_y > 0 && drag == 0 && km_mode == 0) {
                        win_y -= 20;
                        draw_window();
                    }
                    if (ascii_char == 'c') {
                        drag = 0;
                        help_col = 65;
                        draw_window();
                        play_sound(900);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '1' && drag == 0) {
                        bg_col = 0x18C3;
                        theme = 1;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '2' && drag == 0) {
                        theme = 2;
                        bg_col = 0x2000;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }                        
                    if (ascii_char == '3' && drag == 0) {
                        bg_col = 0x1041;
                        theme = 3;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '4' && drag == 0) {
                        theme = 4;
                        bg_col = 0x10A2;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '5' && drag == 0) {
                        bg_col = 0x01C8;
                        theme = 5;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '6' && drag == 0) {
                        bg_col = 0x00A1;
                        theme = 6;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '7' && drag == 0) {
                        bg_col = 0x4083;
                        theme = 7;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'F' && drag == 0) {
                        w_mode = 1;
                        for (int y = win_y + 200; y < win_y + 500; y++) {
                            for (int x = win_x + 20; x < win_x + 720; x++) {
                                if (y == win_y + 200 || y == win_y + 499 || x == win_x + 20|| x == win_x + 719) {
                                    gfx_memory[y * 1024 + x] = 0x0320;
                                }
                                else if (y < win_y + 203) {
                                    gfx_memory[y * 1024 + x] = 0x3DEF;
                                }
                                else if (y < win_y + 209) {
                                    gfx_memory[y * 1024 + x] = 0x24EE;
                                }
                                else if (y < win_y + 215) {
                                    gfx_memory[y * 1024 + x] = 0x11EB;
                                }
                                else {
                                    gfx_memory[y * 1024 + x] = 0xFFFF;
                                }
                            }
                        }
                        int swin_x = win_x + 20;
                        int swin_y = win_y + 200;
                        int swin_w = win_w - 40;
                        gfx_memory[swin_y * 1024 + swin_x] = 0xFFFF;
                        gfx_memory[swin_y * 1024 + (swin_x+1)] = 0xFFFF;
                        gfx_memory[(swin_y+1) * 1024 + swin_x] = 0xFFFF;
                        int right_edges = swin_x + swin_w - 1;
                        gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                        gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                        gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                        print_string("Preview", win_x + 24, win_y + 204, 0xFFFF);
                        print_string(ftext, win_x + 24, win_y + 220, 0x0000);
                        print_string("F2: exit.", win_x + 24, win_y + 230, 0x0000);
                        play_sound(500);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'T' && km_mode == 0 && drag == 0) {
                        km_mode = 1;
                        play_sound(200);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'U' && km_mode == 1 && pos_y > win_y + 30 && drag == 0) {
                        prev_cursor();
                        pos_y -= 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'D' && km_mode == 1 && pos_y < win_y + 515 && drag == 0) {
                        prev_cursor();
                        pos_y += 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'R' && km_mode == 1 && pos_x < win_x + 715 && drag == 0) {
                        prev_cursor();
                        pos_x += 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'L' && km_mode == 1 && pos_x > win_x + 15 && drag == 0) {
                        prev_cursor();
                        pos_x -= 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'G' && km_mode == 1 && drag == 0) {
                        km_mode = 0;
                        play_sound(1000);
                        sleep(100);
                        no_sound();
                        draw_window();
                    }
                    if (ascii_char == 'e' && km_mode == 1 && drag == 0) {
                        if (pos_x >= win_x + 250 && pos_x <= win_x + 290 && pos_y <= win_y + 30)  { pong(); }
                        if (pos_x >= win_x + 310 && pos_x <= win_x + 350 && pos_y <= win_y + 30)  { shutdown(); }
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30  && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            print_string("Help", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Arrows: move win", win_x + 30, help_col, 0x0000);
                            print_string("C: clear and close", win_x + 30, help_col + 15, 0x0000);
                            print_string("1-7: change theme", win_x + 30, help_col + 30, 0x0000);
                            print_string("F1/F2: write", win_x + 30, help_col + 45, 0x0000);
                            print_string("F: format", win_x + 30, help_col + 60, 0x0000);
                            print_string("F3/F4: key-mouse", win_x + 30, help_col + 75, 0x0000);
                        }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && drag == 0) { cpu_win(); }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30 && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            create_file("Unnamed.txt", ftext);
                            print_string(ram_disk[fid].name, win_x + 27, win_y + 27, 0xFFFF);
                            print_string(ram_disk[fid].content, win_x + 30, help_col, 0x0000);
                            print_string("C: close.", win_x + 30, help_col + 15, 0x0000);
                            if (fid <= 4) {
                                fid++;
                            }
                            if (str_in(ftext, "theme1")) {
                                theme = 1;
                                draw_window();
                                drag = 0;
                                bg_col = 0x18C3;
                            }
                            if (str_in(ftext, "theme2")) {
                                theme = 2;
                                draw_window();
                                drag = 0;
                                bg_col = 0x2000;
                            }
                            if (str_in(ftext, "theme3")) {
                                theme = 3;
                                draw_window();
                                drag = 0;
                                bg_col = 0x1041;
                            }
                            if (str_in(ftext, "theme4")) {
                                theme = 4;
                                draw_window();
                                drag = 0;
                                bg_col = 0x10A2;
                            }
                            if (str_in(ftext, "theme5")) {
                                theme = 5;
                                draw_window();
                                drag = 0;
                                bg_col = 0x01C8;
                            }
                            if (str_in(ftext, "theme6")) {
                                theme = 6;
                                draw_window();
                                drag = 0;
                                bg_col = 0x00A1;
                            }
                            if (str_in(ftext, "theme7")) {
                                theme = 7;
                                draw_window();
                                drag = 0;
                                bg_col = 0x4083;
                            }
                            if (str_in(ftext, "format")) {
                                textid = 0;
                                for (int i = 0; i < 99; i++) {
                                    ftext[i] = '\0';}
                                for (int i = 0; i < 5; i++) {
                                    ram_disk[i].exists = 0;
                                    ram_disk[i].size = 0;
                                    for (int n = 0; n < 12; n++) {
                                        ram_disk[i].name[n] = '\0';
                                    }
                                    for (int t = 0; t < 100; t++) {
                                        ram_disk[i].content[t] = '\0';
                                    }
                                }
                                fid = 0;
                            }
                            if (str_in(ftext, "clear")) {
                                drag = 0;
                                help_col = 65;
                                draw_window();
                            }
                            if (str_in(ftext, "winrght")) {
                                drag = 0;
                                win_x += 50;
                                draw_window();
                            }
                            if (str_in(ftext, "winlft")) {
                                drag = 0;
                                win_x -= 50;
                                draw_window();
                            }
                            if (str_in(ftext, "winup")) {
                                drag = 0;
                                win_y -= 50;
                                draw_window();
                            }
                            if (str_in(ftext, "windwn")) {
                                drag = 0;
                                win_y += 50;
                                draw_window();
                            }
                            if (str_in(ftext, "spkr")) {
                                play_sound(750);
                                sleep(250);
                                no_sound();
                            }
                            if (str_in(ftext, "scrblck")) {
                                for (int y = 0; y < 768; y++) {
                                    for (int x = 0; x < 1024; x++) {
                                        gfx_memory[y * 1024 + x] = 0x0000;
                                    }
                                }
                            }
                            if (str_in(ftext, "scrwht")) {
                                for (int y = 0; y < 768; y++) {
                                    for (int x = 0; x < 1024; x++) {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            if (str_in(ftext, "stbusy")) {
                                drag = 1;
                            }
                            if (str_in(ftext, "stfree")) { drag = 0; }
                            if (str_in(ftext, "prntstr")) { print_string(ftext, 480, 359, 0x0000); }
                        }
                        if (pos_x >= win_x + 190 && pos_x <= win_x + 230 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            int line = win_y + 65;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else if (y < win_y + 60) {
                                        gfx_memory[y * 1024 + x] = 0xC618;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            print_string("Explorer", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Name:", win_x + 35, win_y + 45, 0x0000);
                            print_string("Size:", win_w - 35, win_y + 45, 0x0000);
                            for (int i = 0; i < 5; i++) {
                                print_string(ram_disk[i].name, win_x + 30, line, 0x0000);
                                char size_str[16];
                                int_str(ram_disk[i].size, size_str);
                                print_string(size_str, win_w - 30, line, 0x0000);
                                line += 15;
                            }
                        }
                    }
                    if (ascii_char == 'f' && drag == 0) {
                        textid = 0;
                        for (int i = 0; i < 99; i++) {
                            ftext[i] = '\0';}
                        for (int i = 0; i < 5; i++) {
                            ram_disk[i].exists = 0;
                            ram_disk[i].size = 0;
                            for (int n = 0; n < 12; n++) {
                                ram_disk[i].name[n] = '\0';
                            }
                            for (int t = 0; t < 100; t++) {
                                ram_disk[i].content[t] = '\0';
                            }
                        }
                        fid = 0;
                        play_sound(800);
                        sleep(100);
                        no_sound();
                    }
                
                }   
                if (scan_code < 0x80 && w_mode == 1) {
                    char ascii_char = scan_code_to_ascii(scan_code);
                    if (ascii_char != 'F' && ascii_char != 'S' && ascii_char != 'B' && textid < 74) {
                        ftext[textid] = ascii_char;
                        textid++;
                        ftext[textid] = '\0';
                        print_string(ftext, win_x + 24, win_y + 220, 0x0000);
                        print_string("F2: exit.", win_x + 24, win_y + 230, 0x0000);
                    }
                    if (ascii_char == 'S') {
                        w_mode = 0;
                        draw_window();
                        play_sound(300);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'B') {
                        if (textid > 0) {
                            textid--;
                            ftext[textid] = '\0';
                        }
                        for (int y = win_y + 200; y < win_y + 500; y++) {
                            for (int x = win_x + 20; x < win_x + 720; x++) {
                                if (y == win_y + 200 || y == win_y + 499 || x == win_x + 20|| x == win_x + 719) {
                                    gfx_memory[y * 1024 + x] = 0x0320;
                                }
                                else if (y < win_y + 203) {
                                    gfx_memory[y * 1024 + x] = 0x3DEF;
                                }
                                else if (y < win_y + 209) {
                                    gfx_memory[y * 1024 + x] = 0x24EE;
                                }
                                else if (y < win_y + 215) {
                                    gfx_memory[y * 1024 + x] = 0x11EB;
                                }
                                else {
                                    gfx_memory[y * 1024 + x] = 0xFFFF;
                                }
                            }
                        }
                        int swin_x = win_x + 20;
                        int swin_y = win_y + 200;
                        int swin_w = win_w - 40;
                        gfx_memory[swin_y * 1024 + swin_x] = 0xFFFF;
                        gfx_memory[swin_y * 1024 + (swin_x+1)] = 0xFFFF;
                        gfx_memory[(swin_y+1) * 1024 + swin_x] = 0xFFFF;
                        int right_edges = swin_x + swin_w - 1;
                        gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                        gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                        gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                        print_string("Preview", win_x + 24, win_y + 204, 0xFFFF);
                        print_string(ftext, win_x + 24, win_y + 220, 0x0000);
                        print_string("F2: exit.", win_x + 24, win_y + 230, 0x0000);
                    }
                }
            }
        }
    }
}
int score = 0;
void pong() {
    if (pos_x >= win_x + 250 && pos_x <= win_x + 290 && pos_y <= win_y + 30) {
        drag = 1;
        pad_x = win_x + (win_w / 2) - (pad_w / 2);
        pad_y = win_y + win_h - 40;
        ball_x = win_x + (win_w / 2);
        ball_y = win_y + 100;
        collisions = 0;
        ball_dx = 3;
        ball_dy = 3;
        score = 0;
        for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
            for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                if (y == win_y + 22 || y == win_y + 22 + win_h - 41 || x == win_x + 20 || x == win_x + 20 + win_w - 41) {
                    gfx_memory[y * 1024 + x] = 0x0320;
                }
                else if (y < win_y + 25) {
                    gfx_memory[y * 1024 + x] = 0x3DEF;
                }
                else if (y < win_y + 31) {
                    gfx_memory[y * 1024 + x] = 0x24EE;
                }
                else if (y < win_y + 37) {
                    gfx_memory[y * 1024 + x] = 0x11EB;
                }
                else {
                    gfx_memory[y * 1024 + x] = 0xEF59;
                }
            }
        }
        int swin_x = win_x + 20;
        int swin_y = win_y + 22;
        int swin_w = win_w - 40;
        gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
        gfx_memory[swin_y * 1024 + (swin_x + 1)] = 0x0000;
        gfx_memory[(swin_y + 1) * 1024 + swin_x] = 0x0000;
        int right_edges = swin_x + swin_w - 1;
        gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
        gfx_memory[swin_y * 1024 + (right_edges + 1)] = 0xFFFF;
        gfx_memory[(swin_y + 1) * 1024 + right_edges] = 0xFFFF;
        print_string("Pong score:", win_x + 27, win_y + 27, 0xFFFF);
        while (1) {
            unsigned char scan_code = inb(0x60);
            if (scan_code < 0x80 && w_mode == 0) { // KEYBOARD CLICKS
                char ascii_char = scan_code_to_ascii(scan_code);
                if (ascii_char == 'c') {
                    drag = 0;
                    draw_window();
                    draw_cursor(pos_x, pos_y);
                    break;
                }
                if (ascii_char == 'd' && pad_x <= win_x + 630) {
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xEF59;
                        }
                    }
                    pad_x += 20;
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xB269;
                        }
                    }
                    sleep(20);
                }
                if (ascii_char == 'a' && pad_x >= win_x + 50) {
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xEF59;
                        }
                    }
                    pad_x -= 20;
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xB269;
                        }
                    }
                    sleep(20);
                }
            }
            for (int x = 0; x < ball_size; x++) {
                for (int y = 0; y < ball_size; y++) {
                    gfx_memory[(ball_y + y) * 1024 + (ball_x + x)] = 0xEF59;
                }
            }
            ball_x += ball_dx;
            ball_y += ball_dy;
            if (ball_x < win_x + 30) {
                ball_x = swin_x + 10;
                ball_dx = -ball_dx;
                play_sound(500);
                sleep(100);
                no_sound();
            }
            if (ball_x > win_x + 710) {
                ball_x = win_x + 710;
                ball_dx = -ball_dx;
                play_sound(500);
                sleep(100);
                no_sound();
            }
            if (ball_y < swin_y + 30) {
                ball_y = swin_y + 30;
                ball_dy = -ball_dy;
                play_sound(500);
                sleep(100);
                no_sound();
            }
            if (ball_y + ball_size >= pad_y && ball_y <= pad_y + pad_h) {
                if (ball_x + ball_size >= pad_x && ball_x <= pad_x + pad_w) {
                    ball_y = pad_y - ball_size;
                    ball_dy = -ball_dy;
                    play_sound(700);
                    sleep(100);
                    no_sound();
                    collisions++;
                    score++;
                    char str_score[10];
                    int_str(score, str_score);
                    for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                        for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                            if (y == win_y + 22 || y == win_y + 22 + win_h - 41 || x == win_x + 20 || x == win_x + 20 + win_w - 41) {
                                gfx_memory[y * 1024 + x] = 0x0320;
                            }
                            else if (y < win_y + 25) {
                                gfx_memory[y * 1024 + x] = 0x3DEF;
                            }
                            else if (y < win_y + 31) {
                                gfx_memory[y * 1024 + x] = 0x24EE;
                            }
                            else if (y < win_y + 37) {
                                gfx_memory[y * 1024 + x] = 0x11EB;
                            }
                        }
                    }
                    int swin_x = win_x + 20;
                    int swin_y = win_y + 22;
                    int swin_w = win_w - 40;
                    gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                    gfx_memory[swin_y * 1024 + (swin_x + 1)] = 0x0000;
                    gfx_memory[(swin_y + 1) * 1024 + swin_x] = 0x0000;
                    int right_edges = swin_x + swin_w - 1;
                    gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                    gfx_memory[swin_y * 1024 + (right_edges + 1)] = 0xFFFF;
                    gfx_memory[(swin_y + 1) * 1024 + right_edges] = 0xFFFF;
                    print_string("Pong score:", win_x + 27, win_y + 27, 0xFFFF);
                    print_string(str_score, win_x + 147, win_y + 27, 0xFFFF);
                    if (collisions == 5) {
                        collisions = 0;
                        ball_dx -= 1;
                        ball_dy -= 1;
                    }
                }
            }
            if (ball_y > pad_y) {
                pad_x = 500;
                pad_y = 580;
                pad_w = 60;
                pad_h = 20;
                ball_x = 500;
                ball_y = 360;
                drag = 0;
                collisions = 0;
                score = 0;
                ball_dx = 3;
                ball_dy = 3;
                draw_window();
                draw_cursor(pos_x, pos_y);
                play_sound(700);
                sleep(100);
                no_sound();
                play_sound(300);
                sleep(300);
                no_sound();
                break;
            }
            for (int x = 0; x < ball_size; x++) {
                for (int y = 0; y < ball_size; y++) {
                    gfx_memory[(ball_y + y) * 1024 + (ball_x + x)] = 0x7E1F;
                }
            }
            sleep(16);
        }
    }
}
void help() {
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30  && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            print_string("Help", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Arrows: move win", win_x + 30, help_col, 0x0000);
                            print_string("C: clear and close", win_x + 30, help_col + 15, 0x0000);
                            print_string("1-7: change theme", win_x + 30, help_col + 30, 0x0000);
                            print_string("F1/F2: write", win_x + 30, help_col + 45, 0x0000);
                            print_string("F: format", win_x + 30, help_col + 60, 0x0000);
                            print_string("F3/F4: key-mouse", win_x + 30, help_col + 75, 0x0000);
                        }
}
void cpu_win() {
                            if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 1024 + x] = 0x0320;
                                    }
                                   else if (y < win_y + 25) {
                                        gfx_memory[y * 1024 + x] = 0x3DEF;
                                    }
                                    else if (y < win_y + 31) {
                                        gfx_memory[y * 1024 + x] = 0x24EE;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 1024 + x] = 0x11EB;
                                    }
                                    else {
                                        gfx_memory[y * 1024 + x] = 0xFFFF;
                                    }
                                }
                            }
                            int swin_x = win_x + 20;
                            int swin_y = win_y + 22;
                            int swin_w = win_w - 40;
                            gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
                            gfx_memory[swin_y * 1024 + (swin_x+1)] = 0x0000;
                            gfx_memory[(swin_y+1) * 1024 + swin_x] = 0x0000;
                            int right_edges = swin_x + swin_w - 1;
                            gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
                            gfx_memory[swin_y * 1024 + (right_edges+1)] = 0xFFFF;
                            gfx_memory[(swin_y+1) * 1024 + right_edges] = 0xFFFF;
                            char cpu_name[49];
                            get_cpu(cpu_name);
                            print_string("CPU", win_x + 27, win_y + 27, 0xFFFF);
                            print_string(cpu_name, win_x + 30, help_col, 0x0000);
                            print_string("C: close.", win_x + 30, help_col + 15, 0x0000);
                        }
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
            gfx_memory[(y << 10) + x] = 0x0000;
        }
    }
    print_string("maxOS is shutting down...", 400, 384, 0xB269);
    play_sound(800); sleep(150); no_sound();
    play_sound(600); sleep(150); no_sound();
    play_sound(400); sleep(250); no_sound();
    sleep(3000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x0B004, 0x2000);
    play_sound(800); sleep(300); no_sound();
    asm volatile("cli; hlt");
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
int create_file(char* name, char* text) {
    for (int i = 0; i < 5; i++) {
        if (ram_disk[i].exists == 0) {
            int n = 0;
            while (name[n] != '\0' && n < 11) {
                ram_disk[i].name[n] = name[n];
                n++;
            }
            ram_disk[i].name[n] = '\0';
            int t = 0;
            while (text[t] != '\0' && t < 99) {
                ram_disk[i].content[t] = text[t];
                t++;
            }
            ram_disk[i].content[t] = '\0';
            ram_disk[i].size = t;
            ram_disk[i].exists = 1;
            return i;
        }
    }
    return -1;
}
void prev_cursor() {
    int prev_x = pos_x;
    int prev_y = pos_y;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int erase_x = prev_x + x;
            int erase_y = prev_y + y;
            gfx_memory[erase_y * 1024 + erase_x] = 0xFFFF;
        }
    }
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
    print_string("File", txt_pos_x, txt_pos_y, 0x0000);
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
void draw_cursor(int mouse_x, int mouse_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            unsigned char pixel_type = mouse_arrow[y][x];
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x < 1024 && screen_y < 768) {
                if (theme == 1) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFFFF;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 2) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xF800;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 3) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4080;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xB269;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 4) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x3186;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xD69F;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 5) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0168;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0x07FF;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 6) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0200;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0x07E0;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 7) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x60A4;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFDF3;}
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
        case 0x3B: return 'F';
        case 0x3C: return 'S';
        case 0x3D: return 'T';
        case 0x3E: return 'G';
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
        case 0x0E: return 'B';
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
    print_string(h, win_x + 700, win_y + 6, 0xFFFF);
    print_string(":", win_x + 710, win_y + 6, 0xFFFF);
    print_string(m, win_x + 720, win_y + 6, 0xFFFF);
}
void draw_window() {
    for (int y = 0; y < 768; y++) {
        int row_offset = y << 10;
        for (int x = 0; x < 1024; x++) {
            if (theme == 1) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x10A2;
                }
                else { gfx_memory[row_offset + x] = 0x2124; }
            }
            if (theme == 2) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x4002;
                }
                else { gfx_memory[row_offset + x] = 0x8085; }
            }            
            if (theme == 3) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x0102;
                }
                else { gfx_memory[row_offset + x] = 0x2A45; }
            }            
            if (theme == 4) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x1041;
                }
                else { gfx_memory[row_offset + x] = 0x4102; }
            }            
            if (theme == 5) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x010A;
                }
                else { gfx_memory[row_offset + x] = 0x03EF; }
            }
            if (theme == 6) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x0102;
                }
                else { gfx_memory[row_offset + x] = 0x05E0; }
            }            
            if (theme == 7) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x4004;
                }
                else { gfx_memory[row_offset + x] = 0xFBEF; }
            }        }
    }
    for (int y = win_y; y < win_y + win_h; y++) {
        for (int x = win_x; x < win_x + win_w; x++) {
            if (y == win_y || y == win_y + win_h - 1 || x == win_x || x == win_x + win_w - 1) {
                if (theme == 1) {
                    gfx_memory[y * 1024 + x] = 0xC618;}
                if (theme == 2) {
                    gfx_memory[y * 1024 + x] = 0xFFFA;}
                if (theme == 3) { gfx_memory[y * 1024 + x] = 0x7224; }
                if (theme == 4) { gfx_memory[y * 1024 + x] = 0x9CF3; }
                if (theme == 5) { gfx_memory[y * 1024 + x] = 0x0124; }
                if (theme == 6) { gfx_memory[y * 1024 + x] = 0x0200; }
                if (theme == 7) { gfx_memory[y * 1024 + x] = 0x50C3; }
            }
            else if (y < win_y + 3) {
                if (theme == 1) {
                    gfx_memory[y * 1024 + x] = 0x3D7F;}
                if (theme == 2) {
                    gfx_memory[y * 1024 + x] = 0xB269;
                }
                if (theme == 3) { gfx_memory[y * 1024 + x] = 0xD460; }
                if (theme == 4) { gfx_memory[y * 1024 + x] = 0x4A29; }
                if (theme == 5) { gfx_memory[y * 1024 + x] = 0x0DE5; }
                if (theme == 6) { gfx_memory[y * 1024 + x] = 0x05E5; }
                if (theme == 7) { gfx_memory[y * 1024 + x] = 0xFCEF; }
            }
            else if (y < win_y + 9) {
                if (theme == 1) {
                    gfx_memory[y * 1024 + x] = 0x2417;}
                if (theme == 2) {
                    gfx_memory[y * 1024 + x] = 0x81C6;
                }
                if (theme == 3) { gfx_memory[y * 1024 + x] = 0x92E0; }
                if (theme == 4) { gfx_memory[y * 1024 + x] = 0x31C6; }
                if (theme == 5) { gfx_memory[y * 1024 + x] = 0x03EA; }
                if (theme == 6) { gfx_memory[y * 1024 + x] = 0x03E3; }
                if (theme == 7) { gfx_memory[y * 1024 + x] = 0xB9CD; }
            }
            else if (y < win_y + 15) {
                if (theme == 1) {
                    gfx_memory[y * 1024 + x] = 0x110F;}
                if (theme == 2) {
                    gfx_memory[y * 1024 + x] = 0x4924;
                }
                if (theme == 3) { gfx_memory[y * 1024 + x] = 0x51A0; }
                if (theme == 4) { gfx_memory[y * 1024 + x] = 0x1923; }
                if (theme == 5) { gfx_memory[y * 1024 + x] = 0x01A4; }
                if (theme == 6) { gfx_memory[y * 1024 + x] = 0x01E1; }
                if (theme == 7) { gfx_memory[y * 1024 + x] = 0x7186; }
            }
            else {
                gfx_memory[y * 1024 + x] = 0xFFFF;
            }
       }
    }
    gfx_memory[win_y * 1024 + win_x] = bg_col;
    gfx_memory[win_y * 1024 + (win_x+1)] = bg_col;
    gfx_memory[(win_y+1) * 1024 + win_x] = bg_col;
    int right_edge = win_x + win_w - 1;
    gfx_memory[win_y * 1024 + right_edge] = bg_col;
    gfx_memory[win_y * 1024 + (right_edge+1)] = bg_col;
    gfx_memory[(win_y+1) * 1024 + right_edge] = bg_col;
    if (theme == 3) { print_string("maxOS ClockWork Abrikos", win_x + 10, win_y + 5, 0xFFFF); }
    if (theme == 4) { print_string("maxOS ClockWork Tora", win_x + 10, win_y + 5, 0xFFFF); }
    else {
        print_string("maxOS ClockWork", win_x + 10, win_y + 5, 0xFFFF); }
    clock();
    draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
    draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
    draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
    draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
    draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
    draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
    draw_cursor(pos_x, pos_y);
}

void print_string(char* str, int x, int y, unsigned short color) {
    while (*str != 0) {
        draw_char(*str, x, y, color);
        x += 9;
        str++;
    }
}
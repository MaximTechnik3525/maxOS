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
void reboot();
void filew();
void open_explorer();
void error(char* err);
void ata_read_sector(unsigned int lba, unsigned short* buffer);
void ata_write_sector(unsigned int lba, unsigned short* buffer);
void open_file(int sector);
unsigned char bcd_to_binary(unsigned char bcd);
unsigned char read_rtc_register(unsigned char reg);
unsigned short bg_col = 0x18C3;
unsigned short text_col = 0x0000;
unsigned int ram_mb = 0;
char* bootloader = "Unknown";
unsigned short apm_supp = 0;
void sleep(unsigned int ms);
int str_in(char* main_string, char* substring);
void write(char* msg, int sector);
void read(int sector, char* output);
void power();
__attribute__((aligned(4096))) unsigned char audio_buffer[16384];
int init_sb16();
void play(unsigned int sample, unsigned int length);
void sb16_write(unsigned char reg);
unsigned int inl(unsigned short port);
void outl(unsigned short port, unsigned int data);
void pci_scan(int txt_x, int txt_y);
unsigned short cursor_back[12][12] = {0};
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
unsigned char help_icon[12][12] = {
    {0,0,0,4,4,4,4,4,4,0,0,0},
    {0,0,4,1,1,1,1,1,1,4,0,0},
    {0,4,1,1,2,2,2,2,1,1,4,0},
    {4,1,1,2,2,1,2,2,1,1,1,4},
    {4,1,1,1,1,1,2,2,1,1,1,4},
    {4,3,3,3,3,2,2,3,3,3,3,4},
    {4,3,3,3,3,3,3,3,3,3,3,4},
    {4,3,3,3,3,2,2,3,3,3,3,4},
    {0,4,3,3,3,2,2,3,3,3,4,0},
    {0,0,4,3,3,3,3,3,3,4,0,0},
    {0,0,0,4,4,4,4,4,4,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
unsigned char arch_icon[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,1,0},
    {0,1,4,4,4,4,4,4,4,4,1,0},
    {0,1,4,4,4,4,4,4,4,4,1,0},
    {0,1,4,4,4,4,4,4,4,4,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,2,2,0,0,0,0,0},
    {0,0,2,2,2,2,2,2,2,2,0,0},
};
unsigned char note_icon[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,3,3,3,3,3,3,3,3,1,0},
    {0,1,2,2,2,2,2,2,2,2,1,0},
    {0,1,3,3,3,3,3,3,3,3,1,0},
    {0,1,2,2,2,2,2,2,2,2,1,0},
    {0,1,4,4,4,4,4,4,4,4,1,0},
    {0,1,2,2,2,2,2,2,2,2,1,0},
    {0,1,4,4,4,4,4,4,4,4,1,0},
    {0,1,2,2,2,2,2,2,2,2,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
unsigned char exp_icon[12][12] = {
    {0,0,0,4,4,4,4,4,4,0,0,0},
    {0,0,4,1,1,1,1,1,1,4,0,0},
    {0,4,1,1,1,1,1,1,1,1,4,0},
    {4,1,1,1,1,2,2,1,1,1,1,4},
    {4,1,1,1,2,0,0,2,1,1,1,4},
    {4,3,3,2,0,0,0,0,2,3,3,4},
    {4,3,3,3,2,0,0,2,3,3,3,4},
    {4,3,3,3,2,2,2,3,3,3,3,4},
    {0,4,3,3,3,3,3,3,3,3,4,0},
    {0,0,4,3,3,3,3,3,3,4,0,0},
    {0,0,0,4,4,4,4,4,4,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
unsigned char pong_icon[12][12] = {
    {0,0,0,0,0,3,3,3,3,3,0,0},
    {0,0,0,0,0,3,2,2,2,3,0,0},
    {0,0,0,0,0,3,2,2,2,3,0,0},
    {0,0,0,0,0,3,2,2,2,3,0,0},
    {0,0,0,0,0,3,3,3,3,3,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {3,3,3,3,3,3,3,3,3,3,3,3},
    {3,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,1,1,1,1,1,1,1,1,1,3},
    {3,3,3,3,3,3,3,3,3,3,3,3},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
unsigned char off_icon[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,2,0,1,1,0,2,0,0,0},
    {0,0,2,1,2,1,1,2,1,2,0,0},
    {0,2,1,2,0,1,1,0,2,1,2,0},
    {0,2,1,2,0,1,1,0,2,1,2,0},
    {2,3,2,0,0,3,3,0,0,2,3,2},
    {2,3,2,0,0,3,3,0,0,2,3,2},
    {0,2,3,2,0,0,0,0,2,3,2,0},
    {0,2,3,2,2,2,2,2,2,3,2,0},
    {0,0,2,3,3,3,3,3,3,2,0,0},
    {0,0,0,2,2,2,2,2,2,0,0,0},
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
    0x63,0x73,0x7B,0x6F,0x67,0x63,0x63,0x00, // 78 N
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
int win_x = 150;
int win_y = 140;
int win_w = 740;
int win_h = 550;
int pos_x = 512;
int pos_y = 384;
int theme = 5;
int w_mode = 0;
int km_mode = 0;
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
int textid = 0;
char ftext[100] = {0};
int fid = 0;
int tail = 0;
int repeats = 1;
int help_col;
int explorer_opened = 0;
int sectors = 5000;
int createdFiles = 0;
int currentMin = 0;
int power_opened = 0;
void kmain(unsigned long multiboot_info_address, unsigned long magic) {
    struct multiboot_info* mbi = (struct  multiboot_info*) multiboot_info_address;
    _gfx_memory_backend = (unsigned short*)(unsigned long)mbi->framebuffer_addr;
    if (mbi->framebuffer_pitch > 0) { REAL_PITCH = mbi->framebuffer_pitch / 2; }
    unsigned short width = mbi->framebuffer_width;
    unsigned short height = mbi->framebuffer_height;
    ram_mb = (mbi->mem_upper / 1024) + 1;
    if (mbi->flags & (1 << 9)) {
        bootloader = (char*)(unsigned long)mbi->boot_loader_name;
    }
    if (mbi->flags & (1 << 10)) {
        apm_supp = 1;
    }
    for (int i = 0; i < 11; i++) {
        int checkSector = 4999 + i;
        char fileText[77];
        read(checkSector, fileText);
        if (fileText[0] != '\0') {
            sectors++;
        }
    }
    draw_window();
    draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
    draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
    draw_cursor(pos_x, pos_y);
    help_col = win_y + 35;
    init_mouse();
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 390 && y >= 388) {
                gfx_memory[y * 1024 + x] = 0x0DE5;
            }
            else if (y <= 396 && y >= 391) {
                gfx_memory[y * 1024 + x] = 0x03EA;
            }
            else if (y <= 402 && y >= 397) {
                gfx_memory[y * 1024 + x] = 0x01A4;
            }
            else { gfx_memory[y * 1024 + x] = 0x0040; }
        }
    }
    print_string("maxOS is starting up...", 425, 420, 0x05E5);
    print_string("By MaximTechnik3525", 10, 10, 0x05E5);
    sleep(100);
    if (init_sb16() == 1) {
        int period = 16;
        for (int i = 0; i < 16384; i++){
            if ((i % 2048 == 0)) {
                period += 4;
            }
            audio_buffer[i] = (unsigned char)((i % period) * (255 / period));
        }
        play(11025, 16384);
    }
    sleep(1500); draw_window(); drag = 0;
    unsigned char packet[3];
    while(1) {
        unsigned char raw_min = read_rtc_register(0x02);
        int m = bcd_to_binary(raw_min);
        if (currentMin != m && drag == 0) {
            currentMin = m;
            draw_window();
        }

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
                        if (tail == 0) { prev_cursor(); }
                        pos_x += delta_x / 2;
                        pos_y -= delta_y / 2;
                        if (pos_x > 1012) {pos_x = 1012;}
                        if (pos_x < 0) {pos_x = 0;}
                        if (pos_y > 756) {pos_y = 756;}
                        if (pos_y < 0) {pos_y = 0;}
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_expbtn(win_x + 190, win_y + 20, 42, 12, win_x + 190, win_y + 20, 40, 10, win_x + 195, win_y + 22);
                        draw_pongbtn(win_x + 250, win_y + 20, 42, 12, win_x + 250, win_y + 20, 40, 10, win_x + 255, win_y + 22);
                        draw_offbtn(win_x + 310, win_y + 20, 42, 12, win_x + 310, win_y + 20, 40, 10, win_x + 315, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (click == 1) { // MOUSE CLICKS
                        if (pos_x >= win_x + 250 && pos_x <= win_x + 290 && pos_y <= win_y + 30 && pos_y >= win_y + 20)  { pong(); }
                        if (pos_x >= win_x + 310 && pos_x <= win_x + 350 && pos_y <= win_y + 30 && pos_y >= win_y + 20)  { power(); }
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30 && pos_y >= win_y + 20 && pos_x >= win_x + 10) { help(); }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && pos_y >= win_y + 10) { cpu_win(); }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30 && pos_y >= win_y + 20) { filew(); }
                        if (pos_x >= win_x + 190 && pos_x <= win_x + 230 && pos_y <= win_y + 30 && pos_y >= win_y + 20) { open_explorer(); }
                    }
                }   
            }
            else {
                unsigned char check_mouse = inb(0x64);
                if (check_mouse & 0x20) {
                    inb(0x60);
                    continue;
                }
                unsigned char scan_code = inb(0x60);
                if (scan_code < 0x80 && w_mode == 0 && drag != 2) { // KEYBOARD CLICKS
                    char ascii_char = scan_code_to_ascii(scan_code);
                    if (ascii_char == '1' && power_opened == 1) {
                        shutdown();
                        power_opened = 0;
                    }
                    if (ascii_char == '2' && power_opened == 1) {
                        reboot();
                        power_opened = 0;
                    }
                    if (explorer_opened == 1 && ascii_char == '1') {
                        open_file(5000);
                    }
                    if (explorer_opened == 1 && ascii_char == '2') {
                        open_file(5001);
                    }
                    if (explorer_opened == 1 && ascii_char == '3') {
                        open_file(5002);
                    }
                    if (explorer_opened == 1 && ascii_char == '4') {
                        open_file(5003);
                    }
                    if (explorer_opened == 1 && ascii_char == '5') {
                        open_file(5004);
                    }
                    if (explorer_opened == 1 && ascii_char == '6') {
                        open_file(5005);
                    }
                    if (explorer_opened == 1 && ascii_char == '7') {
                        open_file(5006);
                    }
                    if (explorer_opened == 1 && ascii_char == '8') {
                        open_file(5007);
                    }
                    if (explorer_opened == 1 && ascii_char == '9') {
                        open_file(5008);
                    }
                    if (explorer_opened == 1 && ascii_char == '0') {
                        open_file(5009);
                    }
                    if (ascii_char == 'C' && drag == 0 && tail == 0) {
                        tail = 1;
                        play_sound(900);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'O' && drag == 0 && tail == 1) {
                        tail = 0;
                        draw_window();
                        play_sound(800);
                        sleep(100);
                        no_sound();
                    }
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
                    if (ascii_char == 'E') {
                        drag = 0;
                        explorer_opened = 0;
                        power_opened = 0;
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
                    if (ascii_char == '8' && drag == 0) {
                        bg_col = 0x7BE0;
                        theme = 8;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '9' && drag == 0) {
                        bg_col = 0x0110;
                        theme = 9;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '0' && drag == 0) {
                        bg_col = 0x10A2;
                        theme = 10;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'T' && km_mode == 0 && drag == 0) {
                        km_mode = 1;
                        play_sound(200);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'U' && km_mode == 1 && pos_y > 15 && drag == 0) {
                        if (tail == 0) { prev_cursor(); }
                        pos_y -= 15;
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'D' && km_mode == 1 && pos_y < 741 && drag == 0) {
                        if (tail == 0) { prev_cursor(); }
                        pos_y += 15;
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'R' && km_mode == 1 && pos_x < 997 && drag == 0) {
                        if (tail == 0) { prev_cursor(); }
                        pos_x += 15;
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'L' && km_mode == 1 && pos_x > 15 && drag == 0) {
                        if (tail == 0) { prev_cursor(); }
                        pos_x -= 15;
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
                        if (pos_x >= win_x + 250 && pos_x <= win_x + 290 && pos_y <= win_y + 30 && pos_y >= win_y + 20)  { pong(); }
                        if (pos_x >= win_x + 310 && pos_x <= win_x + 350 && pos_y <= win_y + 30 && pos_y >= win_y + 20)  { shutdown(); }
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30  && drag == 0 && pos_x >= win_x + 10 && pos_y >= win_y + 20) { help(); }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && drag == 0 && pos_y >= win_y + 20) { cpu_win(); }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30 && drag == 0 && pos_y >= win_y + 20) { filew(); }
                        if (pos_x >= win_x + 190 && pos_x <= win_x + 230 && pos_y <= win_y + 30 && pos_y >= win_y + 20) { open_explorer(); }
                    }
                    if (ascii_char == 'f' && explorer_opened == 1) {
                        createdFiles = 0;
                        for (int i = 5000; i < 5011; i++) {
                            write("", i);
                        }
                        sectors = 5000;
                        play_sound(1000);
                        sleep(100);
                        no_sound();
                        open_explorer();
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
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        int swin_x = win_x + 20;
        int swin_y = win_y + 22;
        print_string("Pong - score:", win_x + 28, win_y + 28, 0x0000);
        print_string("Pong - score:", win_x + 27, win_y + 27, 0xFFFF);
        while (1) {
            unsigned char status = inb(0x64);
            if (status & 0x01) {
                if (status & 0x20) {
                    inb(0x60);
                    continue;
                }
            }
            unsigned char scan_code = inb(0x60);
            if (scan_code < 0x80 && w_mode == 0) {
                char ascii_char = scan_code_to_ascii(scan_code);
                if (ascii_char == 'E') {
                    drag = 0;
                    draw_window();
                    play_sound(900); sleep(100); no_sound();
                    break;
                }
                if (ascii_char == 'd' && pad_x <= win_x + 630) {
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xF77D;
                        }
                    }
                    pad_x += 20;
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xF800;
                        }
                    }
                    sleep(20);
                }
                if (ascii_char == 'a' && pad_x >= win_x + 50) {
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xF77D;
                        }
                    }
                    pad_x -= 20;
                    for (int x = 0; x < pad_w; x++) {
                        for (int y = 0; y < pad_h; y++) {
                            int screen_x = pad_x + x;
                            int screen_y = pad_y + y;
                            gfx_memory[screen_y * 1024 + screen_x] = 0xF800;
                        }
                    }
                    sleep(20);
                }
            }
            for (int x = 0; x < ball_size; x++) {
                for (int y = 0; y < ball_size; y++) {
                    gfx_memory[(ball_y + y) * 1024 + (ball_x + x)] = 0xF77D;
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
                    print_string("Pong - score:", win_x + 28, win_y + 28, 0x0000);
                    print_string("Pong - score:", win_x + 27, win_y + 27, 0xFFFF);
                    print_string(str_score, win_x + 148, win_y + 28, 0x0000);
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
                    gfx_memory[(ball_y + y) * 1024 + (ball_x + x)] = 0x03FF;
                }
            }
            sleep(16);
        }
    }
}
void open_file(int sector) 
{
    drag = 1;
    explorer_opened = 0;
    char file_output[77];
    read(sector, file_output);
    if (file_output[0] != '\0') {
    if (str_in(file_output, "!mapp!")) 
    {
        repeats = 1;
        if (str_in(file_output, "!mapp!"))
        {
            if (str_in(file_output, "repeat0"))   { repeats = 0; }
            if (str_in(file_output, "repeat5"))   { repeats = 5; }
            if (str_in(file_output, "repeat10"))  { repeats = 10; }
            if (str_in(file_output, "repeat50"))  { repeats = 50; }
            if (str_in(file_output, "repeat100")) { repeats = 100; }

            for (int range = 0; range < repeats; range++) 
            {
                if (str_in(file_output, "waitkey")) 
                {
                    while (1) 
                    {
                        unsigned char scan_code = inb(0x60);
                        if (scan_code < 0x80) 
                        {
                            char ascii_char = scan_code_to_ascii(scan_code);
                            if (ascii_char != '10') { break; }
                        }
                    }
                }
                if (str_in(file_output, "theme1")) 
                {
                    theme = 1;
                    draw_window();
                    bg_col = 0x18C3;
                }
                if (str_in(file_output, "theme2")) 
                {
                    theme = 2;
                    draw_window();
                    bg_col = 0x2000;
                }
                if (str_in(file_output, "theme3")) 
                {
                    theme = 3;
                    draw_window();
                    bg_col = 0x1041;
                }
                if (str_in(file_output, "theme4")) 
                {
                    theme = 4;
                    draw_window();
                    bg_col = 0x10A2;
                }
                if (str_in(file_output, "theme5")) 
                {
                    theme = 5;
                    draw_window();
                    bg_col = 0x01C8;
                }
                if (str_in(file_output, "theme6")) 
                {
                    theme = 6;
                    draw_window();
                    bg_col = 0x05E0;
                }
                if (str_in(file_output, "theme7")) 
                {
                    theme = 7;
                    draw_window();
                    bg_col = 0xFBEF;
                }
                if (str_in(file_output, "theme8")) 
                {
                    theme = 8;
                    draw_window();
                    bg_col = 0x8B04;
                }
                if (str_in(file_output, "theme9")) 
                {
                    theme = 9;
                    draw_window();
                    bg_col = 0x1000;
                }
                if (str_in(file_output, "theme10")) 
                {
                    theme = 10;
                    draw_window();
                    bg_col = 0x0841;
                }
                if (str_in(file_output, "redraw")) 
                {
                    drag = 0;
                    help_col = 65;
                    draw_window();
                }
                if (str_in(file_output, "winr")) 
                {
                    drag = 0;
                    win_x += 50;
                    draw_window();
                }
                if (str_in(file_output, "winl")) 
                {
                    drag = 0;
                    win_x -= 50;
                    draw_window();
                }
                if (str_in(file_output, "winu")) 
                {
                    drag = 0;
                    win_y -= 50;
                    draw_window();
                }
                if (str_in(file_output, "wind")) 
                {
                    drag = 0;
                    win_y += 50;
                    draw_window();
                }
                if (str_in(file_output, "speaker")) 
                {
                    play_sound(750);
                    sleep(250);
                    no_sound();
                }
                if (str_in(file_output, "scrblack")) 
                {
                    for (int y = 0; y < 768; y++) 
                    {
                        for (int x = 0; x < 1024; x++) 
                        {
                            gfx_memory[y * 1024 + x] = 0x0000;
                        }
                    }
                }
                if (str_in(file_output, "scrwhite")) 
                {
                    for (int y = 0; y < 768; y++) 
                    {
                        for (int x = 0; x < 1024; x++) 
                        {
                            gfx_memory[y * 1024 + x] = 0xFFFF;
                        }
                    }
                }
                if (str_in(file_output, "stbusy")) { drag = 1; }
                if (str_in(file_output, "stfree")) { drag = 0; }
                if (str_in(file_output, "stcrit")) { drag = 2; }
                
                if (str_in(file_output, "drawwin")) 
                {
                    for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) 
                    {
                        for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) 
                        {
                            if (y == win_y + 22 || y == win_y + 22 + win_h - 41 || x == win_x + 20 || x == win_x + 20 + win_w - 41) 
                            {
                                gfx_memory[y * 1024 + x] = 0x0320;
                            }
                            else if (y < win_y + 25) 
                            {
                                gfx_memory[y * 1024 + x] = 0x3DEF;
                            }
                            else if (y < win_y + 31) 
                            {
                                gfx_memory[y * 1024 + x] = 0x24EE;
                            }
                            else if (y < win_y + 37) 
                            {
                                gfx_memory[y * 1024 + x] = 0x11EB;
                            }
                            else 
                            {
                                gfx_memory[y * 1024 + x] = 0xF77D;
                            }
                        }
                    }
                    print_string("Application", win_x + 28, win_y + 28, 0x0000);
                    print_string("Application", win_x + 27, win_y + 27, 0xFFFF);
                }
                if (str_in(file_output, "printstring")) { print_string(file_output, 300, 359, 0x0000); }
                if (str_in(file_output, "sleep")) { sleep(2000); }
                if (str_in(file_output, "errscr")) { error("Caused by user programm. Code: 0x03"); }
                if (str_in(file_output, "shutdown")) { shutdown(); }
                if (str_in(file_output, "reboot")) { reboot(); }
                if (str_in(file_output, "trailon")) { tail = 1; }
                if (str_in(file_output, "trailoff")) { tail = 0; }
                
                if (str_in(file_output, "format")) 
                {
                    createdFiles = 0;
                    for (int i = 5000; i < 5011; i++) {
                        write("", i);
                    }
                    sectors = 5000;
                    play_sound(1000);
                    sleep(100);
                    no_sound();
                    open_explorer();
                }
            }
        }
    }
    else
    {
        for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) 
        {
            for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) 
            {
                if (y == win_y + 22 || y == win_y + 22 + win_h - 41 || x == win_x + 20 || x == win_x + 20 + win_w - 41) 
                {
                    gfx_memory[y * 1024 + x] = 0x0320;
                }
                else if (y < win_y + 25) 
                {
                    gfx_memory[y * 1024 + x] = 0x3DEF;
                }
                else if (y < win_y + 31) 
                {
                    gfx_memory[y * 1024 + x] = 0x24EE;
                }
                else if (y < win_y + 37) 
                {
                    gfx_memory[y * 1024 + x] = 0x11EB;
                }
                else 
                {
                    gfx_memory[y * 1024 + x] = 0xFFFF;
                }
            }
        }
        print_string("Text file", win_x + 28, win_y + 28, 0x0000);
        print_string("Text file", win_x + 27, win_y + 27, 0xFFFF);
        print_string(file_output, win_x + 30, win_y + 45, text_col);
        print_string("Press Esc to close this window.", win_x + 30, win_y + 60, 0x0000);
    }
}
else {
        for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) 
        {
            for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) 
            {
                if (y == win_y + 22 || y == win_y + 22 + win_h - 41 || x == win_x + 20 || x == win_x + 20 + win_w - 41) 
                {
                    gfx_memory[y * 1024 + x] = 0x0320;
                }
                else if (y < win_y + 25) 
                {
                    gfx_memory[y * 1024 + x] = 0x3DEF;
                }
                else if (y < win_y + 31) 
                {
                    gfx_memory[y * 1024 + x] = 0x24EE;
                }
                else if (y < win_y + 37) 
                {
                    gfx_memory[y * 1024 + x] = 0x11EB;
                }
                else 
                {
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        print_string("No file", win_x + 28, win_y + 28, 0x0000);
        print_string("No file", win_x + 27, win_y + 27, 0xFFFF);
        print_string("File not found!", win_x + 30, win_y + 45, text_col);
        print_string("Press Esc to close this window.", win_x + 30, win_y + 60, 0x0000);
}
}
int str_cmp(char* str1, char* str2) {
    int i = 0;
    while(str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 0;
        }
        i++;
    }
    if (str1[i] != str2[i]) { return 0; }
    return 1;
}
int shift_p = 0;
void write(char* msg, int sector) {
    unsigned short write_buffer[256] = {0};
    char* msg_ptr = (char*)write_buffer;
    int msg_id = 0;
    while (msg[msg_id] != '\0' && msg_id < 510) {
        msg_ptr[msg_id] = msg[msg_id];
        msg_id++;
    }
    ata_write_sector(sector, write_buffer);
}
void read(int sector, char* output) {
    unsigned short read_buffer[256] = {0};
    ata_read_sector(sector, read_buffer);
    char* disk_ptr = (char*)read_buffer;
    int i = 0;
    while (disk_ptr[i] != '\0' && i < 76) {
        output[i] = disk_ptr[i];
        i++;
    }
    output[i] = '\0';
}
void filew() {
    if (1 != 0) {
        char str[77];
        read((sectors - 1), str);
        int help_col = win_y + 45;
        w_mode = 1;
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
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        if (str_cmp(ftext, str)) {
            print_string("Notepad: saved", win_x + 28, win_y + 28, 0x0000);
            print_string("Notepad: saved", win_x + 27, win_y + 27, 0xFFFF);
        }
        else {
            print_string("Notepad: unsaved", win_x + 28, win_y + 28, 0x0000);
            print_string("Notepad: unsaved", win_x + 27, win_y + 27, 0xFFFF);
        }
        print_string("Press F1 to save and run. Press Esc to exit without saving.", win_x + 30, help_col + 15, 0x0000);
        print_string("Press shift + F1/F2/F3/F4/F5 to change text color. Programm: !mapp!", win_x + 30, help_col + 30, 0x0000);
        print_string(ftext, win_x + 30, help_col, text_col);
        while (1) {
            unsigned char status = inb(0x64);
            if (status & 0x01) {
                if (status & 0x20) {
                    inb(0x60);
                    continue;
                }
                unsigned char scan_code = inb(0x60);
                if (scan_code == 0x2A || scan_code == 0x36) { shift_p = 1; }
                else if (scan_code == 0xAA || scan_code == 0xB6) { shift_p = 0; }
                if (scan_code < 0x80 && w_mode == 1 && drag != 2 && scan_code != 0x2A && scan_code != 0x36 && scan_code != 0xAA && scan_code != 0xB6) {
                    char ascii_char = scan_code_to_ascii(scan_code);
                    if (shift_p == 0 && ascii_char != 'E' && ascii_char != 'F' && ascii_char != 'B' && ascii_char != 'S' && ascii_char != 'U' && ascii_char != 'D' && ascii_char != 'L' && ascii_char != 'R' && ascii_char != 'T' && ascii_char != 'G' && ascii_char != 'C' && ascii_char != 'O' && ascii_char != 'M' && ascii_char != 'N') {
                        if (textid < 76) {
                            ftext[textid] = ascii_char;
                            textid++;
                            ftext[textid] = '\0';
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
                                        gfx_memory[y * 1024 + x] = 0xF77D;
                                    }
                                }
                            }
                            print_string("Notepad", win_x + 28, win_y + 28, 0x0000);
                            print_string("Notepad", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Press F1 to save and run. Press Esc to exit without saving.", win_x + 30, help_col + 15, 0x0000);
                            print_string("Press shift + F1/F2/F3/F4/F5 to change text color. Programm: !mapp!", win_x + 30, help_col + 30, 0x0000);
                            print_string(ftext, win_x + 30, help_col, text_col);
                            read((sectors - 1), str);
                            if (str_cmp(ftext, str)) {
                                print_string("Notepad: saved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: saved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                            else {
                                print_string("Notepad: unsaved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: unsaved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                            print_string(ftext, win_x + 30, help_col, text_col);
                        }
                    }
                    else if (ascii_char == 'B') {
                        if (textid > 0) {
                            textid--;
                            ftext[textid] = '\0';
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
                                        gfx_memory[y * 1024 + x] = 0xF77D;
                                    }
                                }
                            }
                            print_string("Notepad", win_x + 28, win_y + 28, 0x0000);
                            print_string("Notepad", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Press F1 to save and run. Press Esc to exit without saving.", win_x + 30, help_col + 15, 0x0000);
                            print_string("Press shift + F1/F2/F3/F4/F5 to change text color. Programm: !mapp!", win_x + 30, help_col + 30, 0x0000);
                            print_string(ftext, win_x + 30, help_col, text_col);
                            read((sectors - 1), str);
                            if (str_cmp(ftext, str)) {
                                print_string("Notepad: saved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: saved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                            else {
                                print_string("Notepad: unsaved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: unsaved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                        }
                    }
                    if (shift_p == 1 && ascii_char != '00') {
                        if (textid < 76) {
                            if (ascii_char == '=') {
                                ftext[textid] = '+';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '1') {
                                ftext[textid] = '!';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '2') {
                                ftext[textid] = '@';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '3') {
                                ftext[textid] = '#';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '4') {
                                ftext[textid] = '$';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '5') {
                                ftext[textid] = '%';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '6') {
                                ftext[textid] = '^';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '7') {
                                ftext[textid] = '&';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '8') {
                                ftext[textid] = '*';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '9') {
                                ftext[textid] = '(';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == '0') {
                                ftext[textid] = ')';
                                textid++;
                                ftext[textid] = '\0';
                            }
                            if (ascii_char == 'F') {
                                text_col = 0x0000;
                                print_string(ftext, win_x + 30, help_col, text_col);
                            }
                            if (ascii_char == 'S') {
                                text_col = 0x0112;
                                print_string(ftext, win_x + 30, help_col, text_col);
                            }
                            if (ascii_char == 'T') {
                                text_col = 0x9000;
                                print_string(ftext, win_x + 30, help_col, text_col);
                            }
                            if (ascii_char == 'G') {
                                text_col = 0x9400;
                                print_string(ftext, win_x + 30, help_col, text_col);
                            }
                            if (ascii_char == 'C') {
                                text_col = 0x0320;
                                print_string(ftext, win_x + 30, help_col, text_col);
                            }
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
                                        gfx_memory[y * 1024 + x] = 0xF77D;
                                    }
                                }
                            }
                            print_string("Notepad", win_x + 28, win_y + 28, 0x0000);
                            print_string("Notepad", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Press F1 to save and run. Press Esc to exit without saving.", win_x + 30, help_col + 15, 0x0000);
                            print_string("Press shift + F1/F2/F3/F4/F5 to change text color. Programm: !mapp!", win_x + 30, help_col + 30, 0x0000);
                            print_string(ftext, win_x + 30, help_col, text_col);
                            read((sectors - 1), str);
                            if (str_cmp(ftext, str)) {
                                print_string("Notepad: saved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: saved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                            else {
                                print_string("Notepad: unsaved", win_x + 28, win_y + 28, 0x0000);
                                print_string("Notepad: unsaved", win_x + 27, win_y + 27, 0xFFFF);
                            }
                            print_string(ftext, win_x + 30, help_col, text_col);
                        }
                    }
                    else if (ascii_char == 'F') {
                        if (createdFiles < 10) {
                            write(ftext, sectors);
                            play_sound(750); sleep(100); no_sound();
                            w_mode = 0;
                            draw_window();
                            open_file(sectors);
                            sectors++;
                            createdFiles++;
                            break;
                        }
                        else {
                            play_sound(150); sleep(100); no_sound();
                        }
                    }
                    else if (ascii_char == 'E') {
                        play_sound(150); sleep(100); no_sound();
                        w_mode = 0;
                        drag = 0;
                        draw_window();
                        break;
                    }
                }
            }
        }
    }
}

void help() {
    if (pos_x <= win_x + 50 && pos_y <= win_y + 30 && drag == 0) {
        int help_col = win_y + 45;
        drag = 1;
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
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        // int swin_x = win_x + 20;
        // int swin_y = win_y + 22;
        // int swin_w = win_w - 40;
        // gfx_memory[swin_y * 1024 + swin_x] = 0x0000;
        // gfx_memory[swin_y * 1024 + (swin_x + 1)] = 0x0000;
        // gfx_memory[(swin_y + 1) * 1024 + swin_x] = 0x0000;
        // int right_edges = swin_x + swin_w - 1;
        // gfx_memory[swin_y * 1024 + right_edges] = 0xFFFF;
        // gfx_memory[swin_y * 1024 + (right_edges + 1)] = 0xFFFF;
        // gfx_memory[(swin_y + 1) * 1024 + right_edges] = 0xFFFF;
        print_string("Help", win_x + 28, win_y + 28, 0x0000);
        print_string("Help", win_x + 27, win_y + 27, 0xFFFF);
        print_string("Arrows to move window.", win_x + 30, help_col, 0x0000);
        print_string("Esc to redraw desktop and close windows.", win_x + 30, help_col + 15, 0x0000);
        print_string("1-9 to change system theme.", win_x + 30, help_col + 30, 0x0000);
        print_string("F1 to save and run in notepad.", win_x + 30, help_col + 45, 0x0000);
        print_string("F to format disk to maxFS2 in explorer.", win_x + 30, help_col + 60, 0x0000);
        print_string("F3/F4 to enable and disable keyboard mouse mode.", win_x + 30, help_col + 75, 0x0000);
        print_string("F5/F6 to enable and disable mouse trail.", win_x + 30, help_col + 90, 0x0000);
    }
}

void power() {
        int help_col = win_y + 45;
        drag = 1;
        power_opened = 1;
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
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        print_string("Power control", win_x + 28, win_y + 28, 0x0000);
        print_string("Power control", win_x + 27, win_y + 27, 0xFFFF);
        print_string("Press 1 to off pc.", win_x + 30, help_col, 0x0000);
        print_string("Press 2 to reboot pc.", win_x + 30, help_col + 15, 0x0000);
        print_string("This will delete all unsaved data!", win_x + 30, help_col + 40, 0x9000);
        print_string("Press Esc to close this window.", win_x + 30, help_col + 55, 0x0000);
    }

void progressbar(char* file_des, int xend) {
        for (int y = win_y + 410; y < win_y + 425; y++) {
            for (int x = win_x + 300; x < win_x + 450; x++) {
                gfx_memory[y * 1024 + x] = 0x3186;
            }
        }
        for (int y = win_y + 412; y < win_y + 423; y++) {
            for (int x = win_x + 302; x < xend; x++) {
                gfx_memory[y * 1024 + x] = 0x0DE5;
            }
        }
        print_string(file_des, win_x + 280, win_y + 435, 0x0000);
}

void cpu_win() {
    if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && drag == 0) {
        int help_col = win_y + 45;
        drag = 1;
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
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        char cpu_name[49];
        get_cpu(cpu_name);
        char ram[32];
        int_str(ram_mb, ram);
        write("ATA driver and maxFS2 OK!", 5011);
        char fs_msg[30];
        read(5011, fs_msg);
        print_string("System information. Press Esc to close.", win_x + 28, win_y + 28, 0x0000);
        print_string("System information. Press Esc to close.", win_x + 27, win_y + 27, 0xFFFF);
        print_string("Your CPU:", win_x + 30, help_col, 0x0000);
        print_string(cpu_name, win_x + 120, help_col, 0x0000);
        print_string("Total RAM:", win_x + 30, help_col + 15, 0x0000);
        print_string(ram, win_x + 130, help_col + 15, 0x0000);
        print_string("Bootloader:", win_x + 30, help_col + 30, 0x0000);
        print_string(bootloader, win_x + 135, help_col + 30, 0x0000);
        print_string("APM support:", win_x + 30, help_col + 45, 0x0000);
        if (apm_supp == 1) {
            print_string("OK", win_x + 145, help_col + 45, 0x0320);
        }
        else {
            print_string("No", win_x + 145, help_col + 45, 0x9000);
        }
        print_string("Disk:", win_x + 30, help_col + 60, 0x0000);
        if (fs_msg[0] != '\0') {
            print_string(fs_msg, win_x + 86, help_col + 60, 0x0320);
        }
        else {
            print_string("Error", win_x + 90, help_col + 60, 0x9000);
        }
        print_string("OS: maxOS v3.9 official build", win_x + 30, help_col + 75, 0x0000);
        pci_scan(win_x + 30, help_col + 90);
    }
}
void open_explorer() {
    explorer_opened = 1;
    if (pos_x >= win_x + 190 && pos_x <= win_x + 230 && pos_y <= win_y + 30) {
        int help_col = win_y + 45;
        int line = win_y + 65;
        drag = 1;
        explorer_opened = 1;
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
                else if (y < win_y + 60) {
                    gfx_memory[y * 1024 + x] = 0xC618;
                }
                else {
                    gfx_memory[y * 1024 + x] = 0xF77D;
                }
            }
        }
        print_string("Explorer: F to format, Esc to close. Press 1-0 to open the file.", win_x + 28, win_y + 28, 0x0000);
        print_string("Explorer: F to format, Esc to close. Press 1-0 to open the file.", win_x + 27, win_y + 27, 0xFFFF);
        print_string("File:", win_x + 35, win_y + 45, 0x0000);
        print_string("ID/sector:", win_x + 300, win_y + 45, 0x0000);
        print_string("Size:", (win_x + win_h) - 14, win_y + 45, 0x0000);
        int file_id = 1;
        for (int i = 0; i < (sectors - 5000); i++) {
            char str[77];
            int currentFile = 5000 + i;
            read(currentFile, str);
            if (str[0] != '\0') {
                char fileName[11];
                for (int i = 0; i < 10; i++) {
                    fileName[i] = '\0';
                }
                for (int char_id = 0; char_id < 10; char_id++) {
                    if (str[char_id] == '\0') {
                        break;
                    }
                    fileName[char_id] = str[char_id];
                    fileName[10] = '\0';
                }
                print_string(fileName, win_x + 30, line, 0x0000);
            }
            int real_size = 0;
            char real_size_str[16];
            char ss[16];
            read(currentFile, ss);
            for (int char_id = 0; char_id < 77; char_id++) {
                if (ss[char_id] == '\0') { break; }
                real_size++;
            }
            int_str(real_size, real_size_str);
            print_string(real_size_str, (win_x + win_h) - 8, line, 0x0000);
            print_string("b", (win_x + win_h) + 12, line, 0x0000);
            char file_id_str[70];
            char sector_str[10];
            int_str(file_id, file_id_str);
            int_str(currentFile, sector_str);
            print_string(file_id_str, win_x + 300, line, 0x0000);
            print_string("/", win_x + 310, line, 0x0000);
            print_string(sector_str, win_x + 320, line, 0x0000);
            line += 15;
        }
        if (createdFiles == 0) { progressbar("You can save 10 files", win_x + 305); }
        if (createdFiles == 1) { progressbar("You can save 9 files", win_x + 319); }
        if (createdFiles == 2) { progressbar("You can save 8 files", win_x + 334); }
        if (createdFiles == 3) { progressbar("You can save 7 files", win_x + 347); }
        if (createdFiles == 4) { progressbar("You can save 6 files", win_x + 360); }
        if (createdFiles == 5) { progressbar("You can save 5 files", win_x + 373); }
        if (createdFiles == 6) { progressbar("You can save 4 files", win_x + 386); }
        if (createdFiles == 7) { progressbar("You can save 3 files", win_x + 399); }
        if (createdFiles == 8) { progressbar("You can save 2 files", win_x + 412); }
        if (createdFiles == 9) { progressbar("You can save 1 files", win_x + 425); }
        if (createdFiles == 10) { progressbar("You can save 0 files", win_x + 448); }
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
    drag = 2;
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 390 && y >= 388) {
                gfx_memory[y * 1024 + x] = 0xFBEF;
            }
            else if (y <= 396 && y >= 391) {
                gfx_memory[y * 1024 + x] = 0xF800;
            }
            else if (y <= 402 && y >= 397) {
                gfx_memory[y * 1024 + x] = 0x5000;
            }
            else { gfx_memory[y * 1024 + x] = 0x1000; }

        }
    }
    print_string("maxOS is shutting down...", 420, 420, 0xF800);
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(250); no_sound();
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(150); no_sound();
    play_sound(70); sleep(300); no_sound();
    sleep(3000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x0B004, 0x2000);
    error("Cannot use power off ports! Code: 0x01");
}
void reboot() {
    drag = 2;
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            if (y <= 390 && y >= 388) {
                gfx_memory[y * 1024 + x] = 0xFBEF;
            }
            else if (y <= 396 && y >= 391) {
                gfx_memory[y * 1024 + x] = 0xF800;
            }
            else if (y <= 402 && y >= 397) {
                gfx_memory[y * 1024 + x] = 0x5000;
            }
            else { gfx_memory[y * 1024 + x] = 0x1000; }

        }
    }
    print_string("maxOS is rebooting...", 423, 420, 0xF800);
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(250); no_sound();
    play_sound(200); sleep(150); no_sound();
    play_sound(100); sleep(150); no_sound();
    play_sound(70); sleep(300); no_sound();
    sleep(3000);
    outb(0x64, 0xFE);
    volatile unsigned long long idt_pointer = 0;
    __asm__ __volatile__("lidt %0" : : "m" (idt_pointer));
    __asm__ __volatile__("int $0");
    error("Cannot reboot system! Code: 0x02");
}
void error(char* err) {
    drag = 2;
    for (int y = 0; y < 768; y++) {
        for (int x = 0; x < 1024; x++) {
            gfx_memory[y * 1024 + x] = 0x9800;
        }
    }
    print_string("maxOS error!", 471, 11, 0x0000);
    print_string("maxOS error!", 470, 10, 0xFFFF);
    print_string(err, 11, 31, 0x0000);
    print_string(err, 10, 30, 0xFFFF);
    print_string("Restart or off your PC :(", 11, 51, 0x0000);
    print_string("Restart or off your PC :(", 10, 50, 0xFFFF);
    print_string("If this keep happening, try to fix the issues or avoid repeating this actions.", 11, 71, 0x0000);
    print_string("If this keep happening, try to fix the issues or avoid repeating this actions.", 10, 70, 0xFFFF);
    print_string("You can check error description and code, to get more information.", 11, 91, 0x0000);
    print_string("You can check error description and code, to get more information.", 10, 90, 0xFFFF);
    play_sound(100); sleep(250); play_sound(75); sleep(250); play_sound(50); sleep(250); no_sound();
    sleep(3000); reboot();
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
    int prev_x = pos_x;
    int prev_y = pos_y;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int erase_x = prev_x + x;
            int erase_y = prev_y + y;
            if (erase_x < 1024 && erase_y < 768 && erase_x >= 0 && erase_y >= 0) {
                gfx_memory[erase_y * 1024 + erase_x] = cursor_back[y][x];
            }
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
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 20) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = help_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0x2417; }
                else if (pixel_type2 == 2 ) { gfx_memory[screen_y * 1024 + screen_x] = 0xFFFF; }
                else if (pixel_type2 == 3 ) { gfx_memory[screen_y * 1024 + screen_x] = 0x110F; }
                else if (pixel_type2 == 4 ) { gfx_memory[screen_y * 1024 + screen_x] = 0x0000; }
            }
        }
    }
    // for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
    //     for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
    //         gfx_memory[y * 1024 + x] = 0x7BEF;
    //     }
    // }
    // for (int y = btn_y; y < btn_y + btn_h; y++) {
    //     for (int x = btn_x; x < btn_x + btn_w; x++) {
    //         gfx_memory[y * 1024 + x] = 0xC618;
    //     }
    // }
    print_string("Help", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}

void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 82) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = arch_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0x528A; }
                else if (pixel_type2 == 2) { gfx_memory[screen_y * 1024 + screen_x] = 0x3186; }
                else if (pixel_type2 == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0x31A6; }
                else if (pixel_type2 == 4) { gfx_memory[screen_y * 1024 + screen_x] = 0x10A2; }
            }
        }
    }
    print_string("Arch", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 142) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = note_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0xA4AC; }
                else if (pixel_type2 == 2) { gfx_memory[screen_y * 1024 + screen_x] = 0xD612; }
                else if (pixel_type2 == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0xFFBB; }
                else if (pixel_type2 == 4) { gfx_memory[screen_y * 1024 + screen_x] = 0xF737; }
            }
        }
    }
    print_string("Note", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}
void draw_expbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 198) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = exp_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0xCE79; }
                else if (pixel_type2 == 2) { gfx_memory[screen_y * 1024 + screen_x] = 0x4A49; }
                else if (pixel_type2 == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0x7BEF; }
                else if (pixel_type2 == 4) { gfx_memory[screen_y * 1024 + screen_x] = 0x2104; }
            }
        }
    }
    print_string("Exp", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}
void draw_pongbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 263) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = pong_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0xF800; }
                else if (pixel_type2 == 2) { gfx_memory[screen_y * 1024 + screen_x] = 0x03FF; }
                else if (pixel_type2 == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0x0000; }
            }
        }
    }
    print_string("Pong", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}
void draw_offbtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = (win_x + 326) + x;
            int screen_y = (win_y + 22) + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                unsigned char pixel_type2 = off_icon[y][x];
                if (pixel_type2 == 1) { gfx_memory[screen_y * 1024 + screen_x] = 0xF800; }
                else if (pixel_type2 == 2) { gfx_memory[screen_y * 1024 + screen_x] = 0x10A2; }
                else if (pixel_type2 == 3) { gfx_memory[screen_y * 1024 + screen_x] = 0x5000; }
            }
        }
    }
    print_string("Power", txt_pos_x - 5, txt_pos_y + 15, 0x0000);
}
void draw_cursor(int mouse_x, int mouse_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x < 1024 && screen_y < 768 && screen_x >= 0 && screen_y >= 0) {
                cursor_back[y][x] = gfx_memory[screen_y * 1024 + screen_x];
                unsigned char pixel_type = mouse_arrow[y][x];
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
                if (theme == 8) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x4962;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xF621;}
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 9) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x5000;} //0x07FF
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xFBE0;} //0x0126
                    if (pixel_type == 3) {gfx_memory[screen_y * 1024 + screen_x] = 0x9CD3;}
                }
                if (theme == 10) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 1024 + screen_x] = 0x0000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 1024 + screen_x] = 0xCE79;}
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
        case 0x0D: return '=';
        case 0x0C: return '-';
        case 0x4A: return '-';
        case 0x01: return 'E';
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
    unsigned char raw_day = read_rtc_register(0x07);
    unsigned char raw_month = read_rtc_register(0x08);
    int sec = bcd_to_binary(raw_sec), min = bcd_to_binary(raw_min), hour = bcd_to_binary(raw_hour), day = bcd_to_binary(raw_day), month = bcd_to_binary(raw_month);
    char s[4], m[4], h[4], d[4], mo[4];
    int_str(sec, s);
    int_str(min, m);
    int_str(hour, h);
    int_str(day, d);
    int_str(month, mo);
    print_string(h, win_x + 691, win_y + 7, 0x0000);
    print_string(":", win_x + 711, win_y + 7, 0x0000);
    print_string(m, win_x + 721, win_y + 7, 0x0000);
    print_string(h, win_x + 690, win_y + 6, 0xFFFF);
    print_string(":", win_x + 710, win_y + 6, 0xFFFF);
    print_string(m, win_x + 720, win_y + 6, 0xFFFF);
    print_string(d, win_x + 660, win_y + 7, 0x0000);
    print_string(d, win_x + 659, win_y + 6, 0xFFFF);
    print_string(mo, win_x + 630, win_y + 7, 0x0000);
    print_string(mo, win_x + 629, win_y + 6, 0xFFFF);
    print_string("/", win_x + 645, win_y + 7, 0x0000);
    print_string("/", win_x + 644, win_y + 6, 0xFFFF);
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
                    gfx_memory[row_offset + x] = 0x2080;
                }
                else { gfx_memory[row_offset + x] = 0x4100; }
            }            
            if (theme == 4) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x10A2;
                }
                else { gfx_memory[row_offset + x] = 0x2945; }
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
            }
            if (theme == 8) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0xE62F;
                }
                else { gfx_memory[row_offset + x] = 0x8B04; }
            }
            if (theme == 9) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x3800;
                }
                else { gfx_memory[row_offset + x] = 0x1000; }
            }
            if (theme == 10) {
                if (((x ^ y) & 16) == 0) {
                    gfx_memory[row_offset + x] = 0x10A2;
                }
                else { gfx_memory[row_offset + x] = 0x0841; }
            }
        }
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
                if (theme == 8) { gfx_memory[y * 1024 + x] = 0x5A21; }
                if (theme == 9) { gfx_memory[y * 1024 + x] = 0xFC00; }
                if (theme == 10) { gfx_memory[y * 1024 + x] = 0x5AEB; }
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
                if (theme == 8) { gfx_memory[y * 1024 + x] = 0xFFFA; }
                if (theme == 9) { gfx_memory[y * 1024 + x] = 0xFBEF; }
                if (theme == 10) { gfx_memory[y * 1024 + x] = 0x31A6; }
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
                if (theme == 8) { gfx_memory[y * 1024 + x] = 0xE60B; }
                if (theme == 9) { gfx_memory[y * 1024 + x] = 0xF800; }
                if (theme == 10) { gfx_memory[y * 1024 + x] = 0x2124; }
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
                if (theme == 8) { gfx_memory[y * 1024 + x] = 0x9BC5; }
                if (theme == 9) { gfx_memory[y * 1024 + x] = 0x5000; }
                if (theme == 10) { gfx_memory[y * 1024 + x] = 0x10A2; }
            }
            else {
                gfx_memory[y * 1024 + x] = 0xF77D;
            }
       }
    }
    if (theme == 3) {
        print_string("maxOS 3.9 Abrikos", win_x + 11, win_y + 6, 0x0000);
        print_string("maxOS 3.9 Abrikos", win_x + 10, win_y + 5, 0xFFFF); }
    if (theme == 4) {
        print_string("maxOS 3.9 Tora", win_x + 11, win_y + 6, 0x0000);
        print_string("maxOS 3.9 Tora", win_x + 10, win_y + 5, 0xFFFF); }
    else {
        print_string("maxOS 3.9", win_x + 11, win_y + 6, 0x0000); 
        print_string("maxOS 3.9", win_x + 10, win_y + 5, 0xFFFF); }
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

unsigned int inl(unsigned short port) {
    unsigned int result;
    __asm__ __volatile__("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outl(unsigned short port, unsigned int data) {
    __asm__ __volatile__("outl %0, %1" : : "a"(data), "Nd"(port));
}
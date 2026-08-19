struct VirtualFile {
    char name[12];
    int size;
    char content[100];
    int exists;
};
struct VirtualFile ram_disk[5];
void print_string(char* str, int x, int y, unsigned short color);
void draw_char(char c, int start_x, int start_y, char color);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
char scan_code_to_ascii(unsigned char scan_code);
unsigned short* gfx_memory;
void draw_cursor(int mouse_x, int mouse_y);
void draw_btn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y);
void get_cpu(char* buffer);
void draw_window();
void wait_mouse(unsigned char type);
void init_mouse();
void prev_cursor();
void play_sound(unsigned int nfreq);
void no_sound();
void sleep(unsigned int ms);
int str_in(char* main_string, char* substring);
int create_file(char* name, char* text);
unsigned char mouse_arrow[12][12] = {
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,1,1,1,1,0,0,0},
    {1,2,1,1,2,1,0,0,0,0,0,0},
    {1,1,0,0,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0}
};
// Готовый ASCII-шрифт 8x8 (содержит Пробел, знаки, ЦИФРЫ, ЗАГЛАВНЫЕ и строчные буквы)
unsigned char font_data[256][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Добавь эти знаки препинания внутрь font_data:
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00}, // Двоеточие
    ['/'] = {0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // Слэш (косая черта)
    ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // Дефис / Минус
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // Точка

    // ЦИФРЫ (0-9)
    ['0'] = {0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x3C, 0x00},
    ['1'] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['2'] = {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},
    ['3'] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    ['4'] = {0x06, 0x0E, 0x1E, 0x36, 0x7E, 0x06, 0x06, 0x00},
    ['5'] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['6'] = {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    ['7'] = {0x7E, 0x66, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00},
    ['8'] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['9'] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00},

    // ЗАГЛАВНЫЕ БУКВЫ (A-Z)
    ['A'] = {0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C'] = {0x3E, 0x60, 0x60, 0x60, 0x60, 0x60, 0x3E, 0x00},
    ['D'] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E'] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},
    ['F'] = {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},
    ['G'] = {0x3E, 0x60, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00},
    ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['J'] = {0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['K'] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
    ['L'] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    ['M'] = {0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x00},
    ['N'] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    ['O'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['P'] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['Q'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x1E, 0x00},
    ['R'] = {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    ['S'] = {0x3E, 0x60, 0x60, 0x3C, 0x02, 0x02, 0x7C, 0x00},
    ['T'] = {0x7E, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['W'] = {0x81, 0x81, 0x81, 0x99, 0xA5, 0xC3, 0x81, 0x00},
    ['X'] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    ['Z'] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},

    // СТРОЧНЫЕ БУКВЫ (a-z)
    ['a'] = {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x46, 0x3B, 0x00},
    ['b'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    ['c'] = {0x00, 0x00, 0x3C, 0x40, 0x40, 0x42, 0x3C, 0x00},
    ['d'] = {0x06, 0x06, 0x3E, 0x46, 0x46, 0x46, 0x3B, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00},
    ['f'] = {0x1C, 0x22, 0x20, 0x78, 0x20, 0x20, 0x20, 0x00},
    ['g'] = {0x00, 0x00, 0x3B, 0x46, 0x46, 0x3E, 0x06, 0x3C},
    ['h'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i'] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['j'] = {0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x38},
    ['k'] = {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00},
    ['l'] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x1C, 0x00},
    ['m'] = {0x00, 0x00, 0x7C, 0x92, 0x92, 0x92, 0x92, 0x00},
    ['n'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['p'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['q'] = {0x00, 0x00, 0x3E, 0x46, 0x46, 0x3E, 0x06, 0x06},
    ['r'] = {0x00, 0x00, 0x5C, 0x32, 0x20, 0x20, 0x20, 0x00},
    ['s'] = {0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C, 0x00},
    ['t'] = {0x20, 0x20, 0x78, 0x20, 0x20, 0x22, 0x1C, 0x00},
    ['u'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3B, 0x00},
    ['v'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x81, 0x99, 0xA5, 0xE7, 0x81, 0x00},
    ['x'] = {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00},
    ['y'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    ['z'] = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}
};

int win_x = 150;
int win_y = 140;
int win_w = 500;
int win_h = 350;
int pos_x = 400;
int pos_y = 300;
int theme = 1;
int w_mode = 0;
int km_mode = 0;
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
    int drag = 0;
    draw_window();
    draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
    draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
    draw_cursor(pos_x, pos_y);
    int help_col = win_y + 35;
    init_mouse();
    play_sound(100);
    sleep(100);
    no_sound();
    play_sound(300);
    sleep(100);
    no_sound();
    play_sound(500);
    sleep(100);
    no_sound();
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
                        if (pos_x > win_x + 478) {pos_x = win_x + 478;}
                        if (pos_x < win_x + 12) {pos_x = win_x + 12;}
                        if (pos_y > win_y + 328) {pos_y = win_y + 328;}
                        if (pos_y < win_y + 22) {pos_y = win_y + 22;}
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (click == 1) {
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            print_string("Help", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Arrows: move window.", win_x + 30, help_col, 0x0000);
                            print_string("C: clear and close.", win_x + 30, help_col + 15, 0x0000);
                            print_string("1-7: change theme.", win_x + 30, help_col + 30, 0x0000);
                            print_string("F1/F2: write mode.", win_x + 30, help_col + 45, 0x0000);
                            print_string("F: format disk.", win_x + 30, help_col + 60, 0x0000);
                            print_string("F3/F4: key-mouse.", win_x + 30, help_col + 75, 0x0000);
                        }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            char cpu_name[49];
                            get_cpu(cpu_name);
                            print_string("CPU", win_x + 27, win_y + 27, 0xFFFF);
                            print_string(cpu_name, win_x + 30, help_col, 0x0000);
                            print_string("C: close.", win_x + 30, help_col + 15, 0x0000);
                        }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
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
                            }
                            if (str_in(ftext, "theme2")) {
                                theme = 2;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme3")) {
                                theme = 3;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme4")) {
                                theme = 4;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme5")) {
                                theme = 5;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme6")) {
                                theme = 6;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme7")) {
                                theme = 7;
                                draw_window();
                                drag = 0;
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
                                print_string("Disk formated!", 5, 590, 0xFFFF);
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
                                for (int y = 0; y < 600; y++) {
                                    for (int x = 0; x < 800; x++) {
                                        gfx_memory[y * 800 + x] = 0x0000;
                                    }
                                }
                            }
                            if (str_in(ftext, "scrwht")) {
                                for (int y = 0; y < 600; y++) {
                                    for (int x = 0; x < 800; x++) {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            if (str_in(ftext, "stbusy")) {
                                drag = 1;
                            }
                            if (str_in(ftext, "stfree")) { drag = 0; }
                            if (str_in(ftext, "prntstr")) { print_string(ftext, 350, 300, 0x0000); }
                        }
                    }
                }   
            }
            else {
                unsigned char scan_code = inb(0x60);
                if (scan_code < 0x80 && w_mode == 0) {
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
                        theme = 1;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '2' && drag == 0) {
                        theme = 2;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }                        
                    if (ascii_char == '3' && drag == 0) {
                        theme = 3;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '4' && drag == 0) {
                        theme = 4;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '5' && drag == 0) {
                        theme = 5;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '6' && drag == 0) {
                        theme = 6;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == '7' && drag == 0) {
                        theme = 7;
                        draw_window();
                        play_sound(700);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'F' && drag == 0) {
                        w_mode = 1;
                        for (int y = win_y + 200; y < win_y + 300; y++) {
                            for (int x = win_x + 20; x < win_x + 470; x++) {
                                if (y == win_y + 200 || y == win_y + 299 || x == win_x + 20|| x == win_x + 469) {
                                    gfx_memory[y * 800 + x] = 0x0320;
                                }
                                else if (y < win_y + 212) {
                                    gfx_memory[y * 800 + x] = 0x6408;
                                }
                                else {
                                    gfx_memory[y * 800 + x] = 0xFFFF;
                                }
                            }
                        }
                        print_string("Preview", win_x + 24, win_y + 203, 0xFFFF);
                        print_string("Write mode on!", 670, 5, 0xFFFF);
                        print_string(ftext, win_x + 24, win_y + 220, 0x0000);
                        print_string("F2: exit.", win_x + 24, win_y + 230, 0x0000);
                        play_sound(500);
                        sleep(100);
                        no_sound();
                    }
                    if (ascii_char == 'T' && km_mode == 0 && drag == 0) {
                        km_mode = 1;
                        print_string("Keyboard mouse on!", 640, 585, 0xFFFF);
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
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'D' && km_mode == 1 && pos_y < win_y + 315 && drag == 0) {
                        prev_cursor();
                        pos_y += 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'R' && km_mode == 1 && pos_x < win_x + 475 && drag == 0) {
                        prev_cursor();
                        pos_x += 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
                        draw_cursor(pos_x, pos_y);
                    }
                    if (ascii_char == 'L' && km_mode == 1 && pos_x > win_x + 15 && drag == 0) {
                        prev_cursor();
                        pos_x -= 15;
                        draw_cursor(pos_x, pos_y);
                        draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
                        draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
                        draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
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
                        if (pos_x <= win_x + 50 && pos_y <= win_y + 30  && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            print_string("Help", win_x + 27, win_y + 27, 0xFFFF);
                            print_string("Arrows: move window.", win_x + 30, help_col, 0x0000);
                            print_string("C: clear and close.", win_x + 30, help_col + 15, 0x0000);
                            print_string("1-7: change theme.", win_x + 30, help_col + 30, 0x0000);
                            print_string("F1/F2: write mode.", win_x + 30, help_col + 45, 0x0000);
                            print_string("F: format disk.", win_x + 30, help_col + 60, 0x0000);
                            print_string("F3/F4: key-mouse.", win_x + 30, help_col + 75, 0x0000);
                        }
                        if (pos_x >= win_x + 70 && pos_x <= win_x + 110 && pos_y <= win_y + 30 && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            char cpu_name[49];
                            get_cpu(cpu_name);
                            print_string("CPU", win_x + 27, win_y + 27, 0xFFFF);
                            print_string(cpu_name, win_x + 110, help_col, 0x0000);
                            print_string("C: close.", win_x + 30, help_col + 15, 0x0000);
                        }
                        if (pos_x >= win_x + 130 && pos_x <= win_x + 170 && pos_y <= win_y + 30 && drag == 0) {
                            int help_col = win_y + 45;
                            drag = 1;
                            for (int y = win_y + 22; y < win_y + 22 + win_h - 40; y++) {
                                for (int x = win_x + 20; x < win_x + 20 + win_w - 40; x++) {
                                    if (y == win_y + 22|| y == win_y + 22 + win_h - 41 || x == win_x + 20|| x == win_x + 20 + win_w - 41) {
                                        gfx_memory[y * 800 + x] = 0x0320;
                                    }
                                    else if (y < win_y + 37) {
                                        gfx_memory[y * 800 + x] = 0x6408;
                                    }
                                    else {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
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
                            }
                            if (str_in(ftext, "theme2")) {
                                theme = 2;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme3")) {
                                theme = 3;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme4")) {
                                theme = 4;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme5")) {
                                theme = 5;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme6")) {
                                theme = 6;
                                draw_window();
                                drag = 0;
                            }
                            if (str_in(ftext, "theme7")) {
                                theme = 7;
                                draw_window();
                                drag = 0;
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
                                print_string("Disk formated!", 5, 590, 0xFFFF);
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
                                for (int y = 0; y < 600; y++) {
                                    for (int x = 0; x < 800; x++) {
                                        gfx_memory[y * 800 + x] = 0x0000;
                                    }
                                }
                            }
                            if (str_in(ftext, "scrwht")) {
                                for (int y = 0; y < 600; y++) {
                                    for (int x = 0; x < 800; x++) {
                                        gfx_memory[y * 800 + x] = 0xFFFF;
                                    }
                                }
                            }
                            if (str_in(ftext, "stbusy")) {
                                drag = 1;
                            }
                            if (str_in(ftext, "stfree")) { drag = 0; }
                            if (str_in(ftext, "prntstr")) { print_string(ftext, 350, 300, 0x0000); }
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
                        print_string("Disk formated!", 5, 590, 0xFFFF);
                        play_sound(800);
                        sleep(100);
                        no_sound();
                    }
                
                }   
                if (scan_code < 0x80 && w_mode == 1) {
                    char ascii_char = scan_code_to_ascii(scan_code);
                    if (ascii_char != 'F' && ascii_char != 'S' && ascii_char != 'B' && textid < 49) {
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
                        for (int y = win_y + 200; y < win_y + 300; y++) {
                            for (int x = win_x + 20; x < win_x + 470; x++) {
                                if (y == win_y + 200 || y == win_y + 299 || x == win_x + 20|| x == win_x + 469) {
                                    gfx_memory[y * 800 + x] = 0x0320;
                                }
                                else if (y < win_y + 212) {
                                    gfx_memory[y * 800 + x] = 0x6408;
                                }
                                else {
                                    gfx_memory[y * 800 + x] = 0xFFFF;
                                }
                            }
                        }
                        print_string("Preview", win_x + 24, win_y + 203, 0xFFFF);
                        print_string("Write mode on!", 670, 5, 0xFFFF);
                        print_string(ftext, win_x + 24, win_y + 220, 0x0000);
                        print_string("F2: exit.", win_x + 24, win_y + 230, 0x0000);
                    }
                }
            }
        }
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
            print_string("File created!", 5, 5, 0xFFFF);
            return i;
        }
    }
    print_string("File was not created!", 5, 5, 0xFFFF);
    return -1;
}
void prev_cursor() {
    int prev_x = pos_x;
    int prev_y = pos_y;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int erase_x = prev_x + x;
            int erase_y = prev_y + y;
            if (theme != 7) {
                gfx_memory[erase_y * 800 + erase_x] = 0xFFFF;
            }
            else  { gfx_memory[erase_y * 800 + erase_x] = 0x4208; }
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
            gfx_memory[y * 800 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 800 + x] = 0xC618;
        }
    }
    print_string("Help", txt_pos_x, txt_pos_y, 0x0000);
}

void draw_cpubtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 800 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 800 + x] = 0xC618;
        }
    }
    print_string("CPU", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_filebtn(int btn2_x, int btn2_y, int btn2_w, int btn2_h, int btn_x, int btn_y, int btn_w, int btn_h, int txt_pos_x, int txt_pos_y) {
    for (int y = btn2_y; y < btn2_y + btn2_h; y++) {
        for (int x = btn2_x; x < btn2_x + btn2_w; x++) {
            gfx_memory[y * 800 + x] = 0x7BEF;
        }
    }
    for (int y = btn_y; y < btn_y + btn_h; y++) {
        for (int x = btn_x; x < btn_x + btn_w; x++) {
            gfx_memory[y * 800 + x] = 0xC618;
        }
    }
    print_string("File", txt_pos_x, txt_pos_y, 0x0000);
}
void draw_cursor(int mouse_x, int mouse_y) {
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            unsigned char pixel_type = mouse_arrow[y][x];
            int screen_x = mouse_x + x;
            int screen_y = mouse_y + y;
            if (screen_x < 800 && screen_y < 600) {
                if (theme == 1) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x0000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0xFFFF;}
                }
                if (theme == 2) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x4000;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0xF800;}
                }
                if (theme == 3) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x4080;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0xB269;}
                }
                if (theme == 4) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x3186;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0xD69F;}
                }
                if (theme == 5) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x0168;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0x05FF;}
                }
                if (theme == 6) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0x0200;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0x07E0;}
                }
                if (theme == 7) {
                    if (pixel_type == 1) {gfx_memory[screen_y * 800 + screen_x] = 0xFFFF;}
                    if (pixel_type == 2) {gfx_memory[screen_y * 800 + screen_x] = 0x0000;}
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

void draw_char(char c, int start_x, int start_y, char color) {
    unsigned char* bitmap = font_data[(unsigned char)c];
    for (int y = 0; y < 8; y++) {
        unsigned char row = bitmap[y];
        for (int x = 0; x < 8; x++) {
            if (row & (0x80 >> x)) {
                gfx_memory[(start_y + y) * 800 + (start_x + x)] = color;
            }
        }
    }
}

void draw_window() {
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            if (theme == 1) {
                gfx_memory[y * 800 + x] = 0x18C3;
            }
            if (theme == 2) { gfx_memory[y * 800 + x] = 0x2000; }
            if (theme == 3) { gfx_memory[y * 800 + x] = 0x1041; }
            if (theme == 4) { gfx_memory[y * 800 + x] = 0x10A2; }
            if (theme == 5) { gfx_memory[y * 800 + x] = 0x0043; }
            if (theme == 6) { gfx_memory[y * 800 + x] = 0x00A1; }
            if (theme == 7) { gfx_memory[y * 800 + x] = 0x2104; }
        }
    }
    for (int y = win_y; y < win_y + win_h; y++) {
        for (int x = win_x; x < win_x + win_w; x++) {
            if (y == win_y || y == win_y + win_h - 1 || x == win_x || x == win_x + win_w - 1) {
                if (theme == 1) {
                    gfx_memory[y * 800 + x] = 0xC618;}
                if (theme == 2) {
                    gfx_memory[y * 800 + x] = 0xFFFA;}
                if (theme == 3) { gfx_memory[y * 800 + x] = 0x7224; }
                if (theme == 4) { gfx_memory[y * 800 + x] = 0x9CF3; }
                if (theme == 5) { gfx_memory[y * 800 + x] = 0x1BFF; }
                if (theme == 6) { gfx_memory[y * 800 + x] = 0x0200; }
                if (theme == 7) { gfx_memory[y * 800 + x] = 0xE71C; }
            }
            else if (y < win_y + 15) {
                if (theme == 1) {
                    gfx_memory[y * 800 + x] = 0x03EF;}
                if (theme == 2) {
                    gfx_memory[y * 800 + x] = 0xB269;
                }
                if (theme == 3) { gfx_memory[y * 800 + x] = 0xD460; }
                if (theme == 4) { gfx_memory[y * 800 + x] = 0x4A29; }
                if (theme == 5) { gfx_memory[y * 800 + x] = 0x3DFF; }
                if (theme == 6) { gfx_memory[y * 800 + x] = 0x05E5; }
                if (theme == 7) { gfx_memory[y * 800 + x] = 0xEF7D; }
            }
            else {
                if (theme != 7) { gfx_memory[y * 800 + x] = 0xFFFF; }
                else { gfx_memory[y * 800 + x] = 0x4208; }
            }
       }
    }
    if (km_mode == 1) { print_string("Keyboard mouse on!", 640, 585, 0xFFFF); }
    if (theme == 3) { print_string("maxOS 2.4 kernel Abrikos", win_x + 10, win_y + 5, 0xFFFF); }
    if (theme == 4) { print_string("maxOS 2.4 kernel Tora", win_x + 10, win_y + 5, 0xFFFF); }
    if (theme == 7) { print_string("maxOS 2.4 kernel", win_x + 10, win_y + 5, 0x0000); }
    else {
        print_string("maxOS 2.4 kernel", win_x + 10, win_y + 5, 0xFFFF); }
    draw_btn(win_x + 10, win_y + 20, 42, 12, win_x + 10, win_y + 20, 40, 10, win_x + 15, win_y + 22);
    draw_cpubtn(win_x + 70, win_y + 20, 42, 12, win_x + 70, win_y + 20, 40, 10, win_x + 75, win_y + 22);
    draw_filebtn(win_x + 130, win_y + 20, 42, 12, win_x + 130, win_y + 20, 40, 10, win_x + 135, win_y + 22);
    draw_cursor(pos_x, pos_y);
}

void print_string(char* str, int x, int y, unsigned short color) {
    while (*str != 0) {
        draw_char(*str, x, y, color);
        x += 9;
        str++;
    }
}
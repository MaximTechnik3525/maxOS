unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void sleep(unsigned int ms);
#define SB_MIXER_ADDR   0x224
#define SB_MIXER_DATA   0x225
#define SB_RESET    0x226
#define SB_READ_DATA    0x22A
#define SB_WRITE_DATA   0x22C
#define SB_READ_STATUS  0x22E
void sb16_volume() {
    outb(SB_MIXER_ADDR, 0x22);
    outb(SB_MIXER_DATA, 0xFF);
    outb(SB_MIXER_ADDR, 0x04);
    outb(SB_MIXER_DATA, 0xFF);
}
int init_sb16() {
    outb(SB_RESET, 1);
    sleep(5);
    outb(SB_RESET, 0);
    sleep(5);
    int timeout = 100000;
    while (((inb(SB_READ_STATUS) & 0x80) == 0) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) { return 0; }

    if (inb(SB_READ_DATA) == 0xAA) {
        sb16_volume();
        return 1;
    }
    return 0;
}
#define dma1_mask_reg   0x0A
#define dma1_mode_reg   0x0B
#define dma1_clear_ff_reg   0x0C
#define dma1_ch1_addr_reg   0x02
#define dma1_ch1_count_reg   0x03
#define dma1_ch1_page_reg   0x83
void sb16_write(unsigned char reg){
    while (inb(SB_WRITE_DATA) & 0x80);
    outb(SB_WRITE_DATA, reg);
}
void setup_dma(unsigned int addr, unsigned int length){
    unsigned int count = length - 1;
    outb(dma1_mask_reg, 0x04 | 1);
    outb(dma1_clear_ff_reg, 0);
    outb(dma1_mode_reg, 0x48 | 1);
    outb(dma1_ch1_addr_reg, (unsigned char)(addr & 0xFF));
    outb(dma1_ch1_addr_reg, (unsigned char)((addr >> 8) & 0xFF));
    outb(dma1_ch1_page_reg, (unsigned char)((addr >> 16) & 0xFF));
    outb(dma1_ch1_count_reg, (unsigned char)(count & 0xFF));
    outb(dma1_ch1_count_reg, (unsigned char)((count >> 8)& 0xFF));
    outb(dma1_mask_reg, 1);
}
void play(unsigned int sample, unsigned int length){
    extern unsigned char audio_buffer[];
    unsigned int addr = (unsigned int)audio_buffer;
    setup_dma(addr, length);
    sb16_write(0x41);
    sb16_write((unsigned char)((sample >> 8) & 0xFF));
    sb16_write((unsigned char)(sample & 0xFF));
    sb16_write(0xC0);
    sb16_write(0x00);
    unsigned int block_size = length - 1;
    sb16_write((unsigned char)(block_size & 0xFF));
    sb16_write((unsigned char)((block_size >> 8) & 0xFF));
}
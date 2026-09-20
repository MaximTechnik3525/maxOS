unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void sleep(unsigned int ms);
#define ATA_REG_DATA    0x1F0
#define ATA_REG_FEATURES    0x1F1
#define ATA_REG_SECCOUNT 0x1F2
#define ATA_REG_LBA_LO  0x1F3
#define ATA_REG_LBA_MID 0x1F4
#define ATA_REG_LBA_HI  0x1F5
#define ATA_REG_DRIVE   0x1F6
#define ATA_REG_COMMAND 0x1F7
#define ATA_REG_STATUS  0x1F7
//MAIN COMMANDS
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0X30
#define ATA_CMD_CACHE_FLUSH 0xE7
//STATUS FLAGS
#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08
#define ATA_SR_DF   0x20
#define ATA_SR_ERR  0x01

static void ata_wait_bsy() {
    while (inb(ATA_REG_STATUS) & ATA_SR_BSY);
}
static void ata_wait_drq() {
    while (!(inb(ATA_REG_STATUS) & ATA_SR_DRQ));
}
void ata_read_sector(unsigned int lba, unsigned short* buffer) {
    ata_wait_bsy();
    outb(ATA_REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_REG_SECCOUNT, 1);
    outb(ATA_REG_LBA_LO, (unsigned char)lba);
    outb(ATA_REG_LBA_MID, (unsigned char)(lba >> 8));
    outb(ATA_REG_LBA_HI, (unsigned char)(lba >> 16));
    outb(ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    ata_wait_bsy();
    ata_wait_drq();
    for (int i = 0; i < 256; i++) {
        unsigned short data;
        __asm__ __volatile__("inw %1, %0" : "=a"(data) : "Nd"(ATA_REG_DATA));
        buffer[i] = data;
    }
}
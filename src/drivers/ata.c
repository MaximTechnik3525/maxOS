#include "ata.h"

struct ATADevice ata_primary_master = {0, 0, 0, "Not Detected"};

static inline unsigned char ata_inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void ata_outb(unsigned short port, unsigned char data) {
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline unsigned short ata_inw(unsigned short port) {
    unsigned short ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void ata_outw(unsigned short port, unsigned short data) {
    __asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}

static void ata_delay(void) {
    ata_inb(0x3F6);
    ata_inb(0x3F6);
    ata_inb(0x3F6);
    ata_inb(0x3F6);
}

static int ata_wait_bsy(void) {
    int timeout = 100000;
    while ((ata_inb(0x1F7) & 0x80) && --timeout);
    return (timeout > 0) ? 0 : -1;
}

static int ata_wait_drq(void) {
    int timeout = 100000;
    while (!((ata_inb(0x1F7) & 0x08)) && --timeout) {
        if (ata_inb(0x1F7) & 0x01) return -1; // ERR
    }
    return (timeout > 0) ? 0 : -1;
}

int ata_identify(void) {
    ata_primary_master.present = 0;
    ata_primary_master.total_sectors = 0;
    ata_primary_master.size_mb = 0;
    for (int i = 0; i < 41; i++) ata_primary_master.model[i] = '\0';

    // Select master drive
    ata_outb(0x1F6, 0xA0);
    ata_delay();

    ata_outb(0x1F2, 0);
    ata_outb(0x1F3, 0);
    ata_outb(0x1F4, 0);
    ata_outb(0x1F5, 0);

    // Send IDENTIFY command
    ata_outb(0x1F7, 0xEC);
    ata_delay();

    unsigned char status = ata_inb(0x1F7);
    if (status == 0) {
        // Drive does not exist
        return -1;
    }

    if (ata_wait_bsy() != 0) {
        return -1;
    }

    // Check for ATAPI
    unsigned char mid = ata_inb(0x1F4);
    unsigned char hi = ata_inb(0x1F5);
    if (mid == 0x14 && hi == 0xEB) {
        // ATAPI device (CD-ROM)
        return -1;
    }

    if (ata_wait_drq() != 0) {
        return -1;
    }

    unsigned short data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = ata_inw(0x1F0);
    }

    // Model name in words 27..46 (byte-swapped)
    int p = 0;
    for (int i = 27; i <= 46; i++) {
        ata_primary_master.model[p++] = (char)(data[i] >> 8);
        ata_primary_master.model[p++] = (char)(data[i] & 0xFF);
    }
    ata_primary_master.model[40] = '\0';

    // Trim trailing spaces in model name
    int len = 39;
    while (len >= 0 && (ata_primary_master.model[len] == ' ' || ata_primary_master.model[len] == '\0')) {
        ata_primary_master.model[len] = '\0';
        len--;
    }

    // Sector count in words 60..61
    unsigned int sectors = (unsigned int)data[60] | ((unsigned int)data[61] << 16);
    ata_primary_master.total_sectors = sectors;
    ata_primary_master.size_mb = sectors / 2048; // 2048 sectors * 512 = 1MB
    ata_primary_master.present = 1;

    return 0;
}

int ata_init(void) {
    // Disable interrupts by setting nIEN bit in Device Control register
    ata_outb(0x3F6, 0x02);
    return ata_identify();
}

int ata_is_available(void) {
    return ata_primary_master.present;
}

int ata_read_sector(unsigned int lba, unsigned char* buffer) {
    if (!ata_primary_master.present) return -1;
    if (ata_wait_bsy() != 0) return -1;

    ata_outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_outb(0x1F1, 0x00);
    ata_outb(0x1F2, 1);
    ata_outb(0x1F3, (unsigned char)(lba & 0xFF));
    ata_outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    ata_outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    ata_outb(0x1F7, 0x20); // READ SECTORS
    ata_delay();

    if (ata_wait_bsy() != 0) return -1;
    if (ata_wait_drq() != 0) return -1;

    unsigned short* ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = ata_inw(0x1F0);
    }
    return 0;
}

int ata_write_sector(unsigned int lba, const unsigned char* buffer) {
    if (!ata_primary_master.present) return -1;
    if (ata_wait_bsy() != 0) return -1;

    ata_outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_outb(0x1F1, 0x00);
    ata_outb(0x1F2, 1);
    ata_outb(0x1F3, (unsigned char)(lba & 0xFF));
    ata_outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    ata_outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    ata_outb(0x1F7, 0x30); // WRITE SECTORS
    ata_delay();

    if (ata_wait_bsy() != 0) return -1;
    if (ata_wait_drq() != 0) return -1;

    const unsigned short* ptr = (const unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ata_outw(0x1F0, ptr[i]);
    }

    // Flush cache
    ata_outb(0x1F7, 0xE7);
    ata_delay();
    if (ata_wait_bsy() != 0) return -1;

    return 0;
}

int ata_read_sectors(unsigned int lba, int count, unsigned char* buffer) {
    for (int i = 0; i < count; i++) {
        if (ata_read_sector(lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int ata_write_sectors(unsigned int lba, int count, const unsigned char* buffer) {
    for (int i = 0; i < count; i++) {
        if (ata_write_sector(lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int ata_flush(void) {
    if (!ata_primary_master.present) return -1;
    ata_outb(0x1F7, 0xE7);
    ata_delay();
    return ata_wait_bsy();
}

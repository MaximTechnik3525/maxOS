#include "ata.h"
#include "ahci.h"
#include "string.h"
#include "debug.h"
#include "../user/libc/sys/syscall.h"

extern long get_cpl(void);

struct ATADevice ata_primary_master = {0, 0, 0, "Not Detected"};
static struct ATADebugStats ata_debug_stats = {0, 0, 0, 0, 0, 0, 0, "IDLE"};

const struct ATADebugStats* ata_get_debug_stats(void) {
    return &ata_debug_stats;
}

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
    // First, check if native SATA AHCI controller is present on PCI bus
    if (ahci_init()) {
        debug_log("STORAGE", "Native SATA AHCI Controller Active - Using Hardware DMA Engine");
        ahci_device_t* sdev = ahci_get_primary_device();
        if (sdev && sdev->present) {
            ata_primary_master.present = 1;
            ata_primary_master.total_sectors = sdev->total_sectors;
            ata_primary_master.size_mb = sdev->size_mb;
            strncpy(ata_primary_master.model, sdev->model, sizeof(ata_primary_master.model) - 1);
            ata_primary_master.model[sizeof(ata_primary_master.model) - 1] = '\0';
        }
        return 0;
    }

    debug_log("ATA", "Initializing Legacy ATA Controller (Primary Master)...");
    // Disable interrupts by setting nIEN bit in Device Control register
    ata_outb(0x3F6, 0x02);
    int res = ata_identify();
    if (res == 0) {
        debug_log("ATA", "ATA Drive Detected and Initialized");
    } else {
        debug_log("ATA", "ATA Drive Not Detected");
    }

    debug_log("ATA", "Initializing ATAPI CD-ROM...");
    ata_outb(0x376, 0x02); // Disable interrupts for secondary bus too
    if (atapi_init() == 0) {
        debug_log("ATA", "ATAPI CD-ROM Detected");
    }

    return res;
}

int ata_is_available(void) {
    if (get_cpl() == 3) {
        return u_ata_status();
    }
    if (ahci_is_available()) return 1;
    return ata_primary_master.present;
}

int ata_read_sector(unsigned int lba, unsigned char* buffer) {
    if (get_cpl() == 3) {
        return u_ata_read(lba, buffer);
    }

    ata_debug_stats.last_lba = lba;
    ata_debug_stats.last_op[0] = 'R';
    ata_debug_stats.last_op[1] = 'E';
    ata_debug_stats.last_op[2] = 'A';
    ata_debug_stats.last_op[3] = 'D';
    ata_debug_stats.last_op[4] = '\0';

    if (ahci_is_available()) {
        int res = ahci_read_sector(lba, buffer);
        if (res == 0) {
            ata_debug_stats.reads_count++;
            debug_log_ata_event("AHCI-RD", lba, 1, 0);
            return 0;
        }
        ata_debug_stats.errors_count++;
        debug_log_ata_event("AHCI-RDERR", lba, 1, -1);
        return -1;
    }

    if (!ata_primary_master.present) {
        ata_debug_stats.errors_count++;
        debug_log_ata_event("RD-NOPRES", lba, 1, -1);
        return -1;
    }
    if (ata_wait_bsy() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("RD-TIMEOUT", lba, 1, -1);
        return -1;
    }

    ata_outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_outb(0x1F1, 0x00);
    ata_outb(0x1F2, 1);
    ata_outb(0x1F3, (unsigned char)(lba & 0xFF));
    ata_outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    ata_outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    ata_outb(0x1F7, 0x20); // READ SECTORS
    ata_delay();

    if (ata_wait_bsy() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("RD-BSYERR", lba, 1, -1);
        return -1;
    }
    if (ata_wait_drq() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("RD-DRQERR", lba, 1, -1);
        return -1;
    }

    unsigned short* ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = ata_inw(0x1F0);
    }

    ata_debug_stats.reads_count++;
    ata_debug_stats.last_status = ata_inb(0x1F7);
    debug_log_ata_event("READ", lba, 1, 0);
    return 0;
}

int ata_write_sector(unsigned int lba, const unsigned char* buffer) {
    if (get_cpl() == 3) {
        return u_ata_write(lba, buffer);
    }

    ata_debug_stats.last_lba = lba;
    ata_debug_stats.last_op[0] = 'W';
    ata_debug_stats.last_op[1] = 'R';
    ata_debug_stats.last_op[2] = 'I';
    ata_debug_stats.last_op[3] = 'T';
    ata_debug_stats.last_op[4] = 'E';
    ata_debug_stats.last_op[5] = '\0';

    if (ahci_is_available()) {
        int res = ahci_write_sector(lba, buffer);
        if (res == 0) {
            ata_debug_stats.writes_count++;
            debug_log_ata_event("AHCI-WR", lba, 1, 0);
            return 0;
        }
        ata_debug_stats.errors_count++;
        debug_log_ata_event("AHCI-WRERR", lba, 1, -1);
        return -1;
    }

    if (!ata_primary_master.present) {
        ata_debug_stats.errors_count++;
        debug_log_ata_event("WR-NOPRES", lba, 1, -1);
        return -1;
    }
    if (ata_wait_bsy() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("WR-TIMEOUT", lba, 1, -1);
        return -1;
    }

    ata_outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_outb(0x1F1, 0x00);
    ata_outb(0x1F2, 1);
    ata_outb(0x1F3, (unsigned char)(lba & 0xFF));
    ata_outb(0x1F4, (unsigned char)((lba >> 8) & 0xFF));
    ata_outb(0x1F5, (unsigned char)((lba >> 16) & 0xFF));
    ata_outb(0x1F7, 0x30); // WRITE SECTORS
    ata_delay();

    if (ata_wait_bsy() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("WR-BSYERR", lba, 1, -1);
        return -1;
    }
    if (ata_wait_drq() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("WR-DRQERR", lba, 1, -1);
        return -1;
    }

    const unsigned short* ptr = (const unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ata_outw(0x1F0, ptr[i]);
    }

    // Flush cache
    ata_outb(0x1F7, 0xE7);
    ata_delay();
    if (ata_wait_bsy() != 0) {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("WR-FLUSHERR", lba, 1, -1);
        return -1;
    }

    ata_debug_stats.writes_count++;
    ata_debug_stats.last_status = ata_inb(0x1F7);
    debug_log_ata_event("WRITE", lba, 1, 0);
    return 0;
}

int ata_read_sectors(unsigned int lba, int count, unsigned char* buffer) {
    if (ahci_is_available()) {
        int res = ahci_read_sectors(lba, count, buffer);
        if (res == 0) {
            ata_debug_stats.reads_count += count;
            return 0;
        }
        return -1;
    }
    for (int i = 0; i < count; i++) {
        if (ata_read_sector(lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int ata_write_sectors(unsigned int lba, int count, const unsigned char* buffer) {
    if (ahci_is_available()) {
        int res = ahci_write_sectors(lba, count, buffer);
        if (res == 0) {
            ata_debug_stats.writes_count += count;
            return 0;
        }
        return -1;
    }
    for (int i = 0; i < count; i++) {
        if (ata_write_sector(lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int ata_flush(void) {
    if (get_cpl() == 3) {
        return u_ata_flush();
    }

    if (ahci_is_available()) {
        ata_debug_stats.flushes_count++;
        return 0;
    }

    ata_debug_stats.last_op[0] = 'F';
    ata_debug_stats.last_op[1] = 'L';
    ata_debug_stats.last_op[2] = 'U';
    ata_debug_stats.last_op[3] = 'S';
    ata_debug_stats.last_op[4] = 'H';
    ata_debug_stats.last_op[5] = '\0';

    if (!ata_primary_master.present) return -1;
    ata_outb(0x1F7, 0xE7);
    ata_delay();
    int ret = ata_wait_bsy();
    if (ret == 0) {
        ata_debug_stats.flushes_count++;
        debug_log_ata_event("FLUSH", 0, 0, 0);
    } else {
        ata_debug_stats.errors_count++;
        ata_debug_stats.last_status = ata_inb(0x1F7);
        ata_debug_stats.last_error_reg = ata_inb(0x1F1);
        debug_log_ata_event("FLUSH-ERR", 0, 0, -1);
    }
    return ret;
}

struct ATAPIDevice atapi_device = {0, 0, 0, "Not Detected", 0, 0};

static void atapi_delay(unsigned short base_port) {
    ata_inb(base_port + 7);
    ata_inb(base_port + 7);
    ata_inb(base_port + 7);
    ata_inb(base_port + 7);
}

static int atapi_wait_bsy(unsigned short base_port) {
    int timeout = 100000;
    while ((ata_inb(base_port + 7) & 0x80) && --timeout);
    return (timeout > 0) ? 0 : -1;
}

static int atapi_wait_drq(unsigned short base_port) {
    int timeout = 100000;
    while (!((ata_inb(base_port + 7) & 0x08)) && --timeout) {
        if (ata_inb(base_port + 7) & 0x01) return -1;
    }
    return (timeout > 0) ? 0 : -1;
}

int atapi_identify_drive(unsigned short base_port, unsigned char drive_sel) {
    ata_outb(base_port + 6, drive_sel);
    atapi_delay(base_port);

    ata_outb(base_port + 2, 0);
    ata_outb(base_port + 3, 0);
    ata_outb(base_port + 4, 0);
    ata_outb(base_port + 5, 0);

    ata_outb(base_port + 7, 0xA1); // IDENTIFY PACKET DEVICE
    atapi_delay(base_port);

    unsigned char status = ata_inb(base_port + 7);
    if (status == 0) return -1;

    if (atapi_wait_bsy(base_port) != 0) return -1;

    // Check signature
    unsigned char mid = ata_inb(base_port + 4);
    unsigned char hi = ata_inb(base_port + 5);
    if (mid != 0x14 || hi != 0xEB) {
        return -1; // Not ATAPI
    }

    if (atapi_wait_drq(base_port) != 0) return -1;

    unsigned short data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = ata_inw(base_port + 0);
    }

    atapi_device.base_port = base_port;
    atapi_device.drive_sel = drive_sel;
    atapi_device.present = 1;

    int p = 0;
    for (int i = 27; i <= 46; i++) {
        atapi_device.model[p++] = (char)(data[i] >> 8);
        atapi_device.model[p++] = (char)(data[i] & 0xFF);
    }
    atapi_device.model[40] = '\0';

    int len = 39;
    while (len >= 0 && (atapi_device.model[len] == ' ' || atapi_device.model[len] == '\0')) {
        atapi_device.model[len] = '\0';
        len--;
    }

    return 0;
}

int atapi_init(void) {
    atapi_device.present = 0;
    // Probe Sec Master
    if (atapi_identify_drive(0x170, 0xA0) == 0) return 0;
    // Probe Sec Slave
    if (atapi_identify_drive(0x170, 0xB0) == 0) return 0;
    // Probe Pri Slave
    if (atapi_identify_drive(0x1F0, 0xB0) == 0) return 0;
    // Probe Pri Master
    if (atapi_identify_drive(0x1F0, 0xA0) == 0) return 0;
    
    return -1;
}

int atapi_read_sector(unsigned int lba, unsigned char* buffer) {
    if (!atapi_device.present) return -1;
    unsigned short base = atapi_device.base_port;

    ata_outb(base + 6, atapi_device.drive_sel);
    atapi_delay(base);

    ata_outb(base + 1, 0); // Features = 0
    ata_outb(base + 4, (2048 & 0xFF)); // byte count limit
    ata_outb(base + 5, (2048 >> 8));

    ata_outb(base + 7, 0xA0); // PACKET command
    atapi_delay(base);

    if (atapi_wait_bsy(base) != 0) return -1;
    
    // Wait for DRQ to send packet
    if (atapi_wait_drq(base) != 0) return -1;

    unsigned char packet[12] = {0};
    packet[0] = 0xA8; // READ(12)
    packet[2] = (lba >> 24) & 0xFF;
    packet[3] = (lba >> 16) & 0xFF;
    packet[4] = (lba >> 8) & 0xFF;
    packet[5] = lba & 0xFF;
    packet[9] = 1; // 1 sector (2048 bytes)

    unsigned short* pkt_words = (unsigned short*)packet;
    for (int i = 0; i < 6; i++) {
        ata_outw(base + 0, pkt_words[i]);
    }

    if (atapi_wait_bsy(base) != 0) return -1;
    if (atapi_wait_drq(base) != 0) return -1;

    unsigned short* ptr = (unsigned short*)buffer;
    for (int i = 0; i < 1024; i++) {
        ptr[i] = ata_inw(base + 0);
    }

    if (atapi_wait_bsy(base) != 0) return -1;
    
    return 0;
}

int atapi_get_volume_label(char* out_label) {
    if (!atapi_device.present) {
        char* str = "No CD-ROM";
        while (*str) *out_label++ = *str++;
        *out_label = '\0';
        return -1;
    }
    unsigned char sector[2048];
    if (atapi_read_sector(16, sector) != 0) {
        char* str = "Read Error";
        while (*str) *out_label++ = *str++;
        *out_label = '\0';
        return -1;
    }
    if (sector[0] != 1 || sector[1] != 'C' || sector[2] != 'D' || sector[3] != '0' || sector[4] != '0' || sector[5] != '1') {
        char* str = "Not ISO9660";
        while (*str) *out_label++ = *str++;
        *out_label = '\0';
        return -1;
    }
    int p = 0;
    for (int i = 0; i < 32; i++) {
        char c = sector[40 + i];
        if (c >= 32 && c <= 126) {
            out_label[p++] = c;
        }
    }
    while (p > 0 && out_label[p-1] == ' ') p--;
    out_label[p] = '\0';
    if (p == 0) {
        char* str = "Unnamed CD";
        while (*str) *out_label++ = *str++;
        *out_label = '\0';
    }
    return 0;
}

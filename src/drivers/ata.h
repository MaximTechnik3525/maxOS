#ifndef ATA_H
#define ATA_H

#define ATA_SECTOR_SIZE 512

struct ATADevice {
    int present;
    unsigned int total_sectors;
    unsigned int size_mb;
    char model[41];
};

struct ATADebugStats {
    unsigned int reads_count;
    unsigned int writes_count;
    unsigned int flushes_count;
    unsigned int errors_count;
    unsigned int last_lba;
    unsigned char last_status;
    unsigned char last_error_reg;
    char last_op[8];
};

extern struct ATADevice ata_primary_master;

int ata_init(void);
int ata_identify(void);
int ata_read_sector(unsigned int lba, unsigned char* buffer);
int ata_write_sector(unsigned int lba, const unsigned char* buffer);
int ata_read_sectors(unsigned int lba, int count, unsigned char* buffer);
int ata_write_sectors(unsigned int lba, int count, const unsigned char* buffer);
int ata_flush(void);
int ata_is_available(void);
const struct ATADebugStats* ata_get_debug_stats(void);

#define ATAPI_SECTOR_SIZE 2048

struct ATAPIDevice {
    int present;
    unsigned int total_sectors;
    unsigned int size_mb;
    char model[41];
    unsigned short base_port;
    unsigned char drive_sel;
};

extern struct ATAPIDevice atapi_device;

int atapi_init(void);
int atapi_identify_drive(unsigned short base_port, unsigned char drive_sel);
int atapi_read_sector(unsigned int lba, unsigned char* buffer);
int atapi_get_volume_label(char* out_label);

#endif // ATA_H

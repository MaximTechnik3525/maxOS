#ifndef ATA_H
#define ATA_H

#define ATA_SECTOR_SIZE 512

struct ATADevice {
    int present;
    unsigned int total_sectors;
    unsigned int size_mb;
    char model[41];
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

#endif // ATA_H

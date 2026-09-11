#ifndef AHCI_H
#define AHCI_H

#include "pci.h"

#define SATA_SIG_ATA    0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive (CD-ROM)
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM     0x96690101  // Port multiplier

#define AHCI_DEV_NULL   0
#define AHCI_DEV_SATA   1
#define AHCI_DEV_SEMB   2
#define AHCI_DEV_PM     3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

// FIS Types
#define FIS_TYPE_REG_H2D   0x27    // Register FIS - host to device
#define FIS_TYPE_REG_D2H   0x34    // Register FIS - device to host
#define FIS_TYPE_DMA_ACT   0x39    // DMA activate FIS - device to host
#define FIS_TYPE_DMA_SETUP 0x41    // DMA setup FIS - bidirectional
#define FIS_TYPE_DATA      0x46    // Data FIS - bidirectional
#define FIS_TYPE_BIST      0x58    // BIST activate FIS - bidirectional
#define FIS_TYPE_PIO_SETUP 0x5F    // PIO setup FIS - device to host
#define FIS_TYPE_DEV_BITS  0xA1    // Set device bits FIS - device to host

// ATA Commands
#define ATA_CMD_READ_DMA_EXT   0x25
#define ATA_CMD_WRITE_DMA_EXT  0x35
#define ATA_CMD_IDENTIFY       0xEC
#define ATA_CMD_IDENTIFY_PACKET 0xA1

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000
#define HBA_PxIS_TFES   (1 << 30)

// Register FIS - Host to Device (20 bytes)
typedef struct {
    unsigned char  fis_type;   // FIS_TYPE_REG_H2D
    unsigned char  pmport_c;   // Port multiplier (bits 0-3), 1: Command, 0: Control (bit 7)
    unsigned char  command;    // Command register
    unsigned char  featurel;   // Feature register, 7:0

    unsigned char  lba0;       // LBA low register, 7:0
    unsigned char  lba1;       // LBA mid register, 15:8
    unsigned char  lba2;       // LBA high register, 23:16
    unsigned char  device;     // Device register

    unsigned char  lba3;       // LBA register, 31:24
    unsigned char  lba4;       // LBA register, 39:32
    unsigned char  lba5;       // LBA register, 47:40
    unsigned char  featureh;   // Feature register, 15:8

    unsigned char  countl;     // Count register, 7:0
    unsigned char  counth;     // Count register, 15:8
    unsigned char  icc;        // Isochronous command completion
    unsigned char  control;    // Control register

    unsigned char  rsv1[4];    // Reserved
} __attribute__((packed)) fis_reg_h2d_t;

// HBA Port Registers (Offset 0x100 + port * 0x80)
typedef volatile struct {
    unsigned int   clb;        // 0x00, command list base address, 1K-byte aligned
    unsigned int   clbu;       // 0x04, command list base address upper 32 bits
    unsigned int   fb;         // 0x08, FIS base address, 256-byte aligned
    unsigned int   fbu;        // 0x0C, FIS base address upper 32 bits
    unsigned int   is;         // 0x10, interrupt status
    unsigned int   ie;         // 0x14, interrupt enable
    unsigned int   cmd;        // 0x18, command and status
    unsigned int   rsv0;       // 0x1C, Reserved
    unsigned int   tfd;        // 0x20, task file data
    unsigned int   sig;        // 0x24, signature
    unsigned int   ssts;       // 0x28, SATA status (SCR0:SStatus)
    unsigned int   sctl;       // 0x2C, SATA control (SCR2:SControl)
    unsigned int   serr;       // 0x30, SATA error (SCR1:SError)
    unsigned int   sact;       // 0x34, SATA active (SCR3:SActive)
    unsigned int   ci;         // 0x38, command issue
    unsigned int   sntf;       // 0x3C, SATA notification (SCR4:SNotification)
    unsigned int   fbs;        // 0x40, FIS-based switch control
    unsigned int   rsv1[11];   // 0x44 ~ 0x6F, Reserved
    unsigned int   vendor[4];  // 0x70 ~ 0x7F, vendor specific
} __attribute__((packed)) hba_port_t;

// Generic Host Control Registers
typedef volatile struct {
    unsigned int   cap;        // 0x00, Host capability
    unsigned int   ghc;        // 0x04, Global host control
    unsigned int   is;         // 0x08, Interrupt status
    unsigned int   pi;         // 0x0C, Port implemented
    unsigned int   vs;         // 0x10, Version
    unsigned int   ccc_ctl;    // 0x14, Command completion coalescing control
    unsigned int   ccc_pts;    // 0x18, Command completion coalescing ports
    unsigned int   em_loc;     // 0x1C, Enclosure management location
    unsigned int   em_ctl;     // 0x20, Enclosure management control
    unsigned int   cap2;       // 0x24, Host capabilities extended
    unsigned int   bohc;       // 0x28, BIOS/OS handoff control and status
    unsigned char  rsv[0xA0-0x2C]; // 0x2C - 0x9F, Reserved
    unsigned char  vendor[0x100-0xA0]; // 0xA0 - 0xFF, Vendor specific
    hba_port_t     ports[32];  // 0x100 - 0x10FF, Port control registers
} __attribute__((packed)) hba_mem_t;

// PRDT (Physical Region Descriptor Table) Entry
typedef struct {
    unsigned int   dba;        // Data base address
    unsigned int   dbau;       // Data base address upper 32 bits
    unsigned int   rsv0;       // Reserved
    unsigned int   dbc:22;     // Byte count, 4M max, 0-based (count - 1)
    unsigned int   rsv1:9;     // Reserved
    unsigned int   i:1;        // Interrupt on completion
} __attribute__((packed)) hba_prdt_entry_t;

// Command Header (32 bytes)
typedef struct {
    unsigned char  cfl:5;      // Command FIS length in DWORDS, 2 ~ 16
    unsigned char  a:1;        // ATAPI
    unsigned char  w:1;        // Write, 1: H2D, 0: D2H
    unsigned char  p:1;        // Prefetchable

    unsigned char  r:1;        // Reset
    unsigned char  b:1;        // BIST
    unsigned char  c:1;        // Clear busy upon R_OK
    unsigned char  rsv0:1;     // Reserved
    unsigned char  pmp:4;      // Port multiplier port

    unsigned short prdtl;      // Physical region descriptor table length in entries

    volatile unsigned int prdbc; // Physical region descriptor byte count transferred

    unsigned int   ctba;       // Command table descriptor base address
    unsigned int   ctbau;      // Command table descriptor base address upper 32 bits

    unsigned int   rsv1[4];    // Reserved
} __attribute__((packed)) hba_cmd_header_t;

// Command Table
typedef struct {
    unsigned char  cfis[64];   // Command FIS
    unsigned char  acmd[16];   // ATAPI command, 12 or 16 bytes
    unsigned char  rsv[48];    // Reserved
    hba_prdt_entry_t prdt_entry[8]; // Up to 8 PRD entries
} __attribute__((packed)) hba_cmd_tbl_t;

// Port Memory Structures Wrapper
typedef struct {
    hba_cmd_header_t* cmd_list;
    void*             fis_base;
    hba_cmd_tbl_t*    cmd_tbl;
    void*             dma_buffer;
} ahci_port_mem_t;

// AHCI Device state
typedef struct {
    int present;
    int port_num;
    hba_port_t* port;
    int type;                  // AHCI_DEV_SATA, etc.
    unsigned int total_sectors;
    unsigned int size_mb;
    char model[41];
    char serial[21];
} ahci_device_t;

extern ahci_device_t ahci_devices[32];
extern int ahci_device_count;
extern int ahci_active_drive;

// API
int  ahci_init(void);
int  ahci_is_available(void);
int  ahci_read_sector(unsigned int lba, unsigned char* buffer);
int  ahci_write_sector(unsigned int lba, const unsigned char* buffer);
int  ahci_read_sectors(unsigned int lba, int count, unsigned char* buffer);
int  ahci_write_sectors(unsigned int lba, int count, const unsigned char* buffer);
ahci_device_t* ahci_get_primary_device(void);

#endif // AHCI_H

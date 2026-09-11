#include "ahci.h"
#include "debug.h"
#include "string.h"
#include "kernel.h"
#include "pmm.h"
#include "pci.h"

static hba_mem_t* hba_base = 0;
ahci_device_t ahci_devices[32];
int ahci_device_count = 0;
int ahci_active_drive = -1;
static ahci_port_mem_t port_mems[32];

static void port_stop(hba_port_t* port) {
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;

    int timeout = 100000;
    while (timeout--) {
        if ((port->cmd & HBA_PxCMD_FR) || (port->cmd & HBA_PxCMD_CR)) {
            continue;
        }
        break;
    }
}

static void port_start(hba_port_t* port) {
    int timeout = 100000;
    while ((port->cmd & HBA_PxCMD_CR) && timeout--);

    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

static int check_type(hba_port_t* port) {
    unsigned int ssts = port->ssts;
    unsigned char ipm = (ssts >> 8) & 0x0F;
    unsigned char det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT || ipm != HBA_PORT_IPM_ACTIVE) {
        return AHCI_DEV_NULL;
    }

    switch (port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_ATA:
        default:
            return AHCI_DEV_SATA;
    }
}

static int ahci_config_port(int port_idx, hba_port_t* port) {
    port_stop(port);

    // Allocate 4KB page for Command List (requires 1KB alignment)
    hba_cmd_header_t* cmd_list = (hba_cmd_header_t*)pmm_alloc_page();
    if (!cmd_list) return 0;
    memset(cmd_list, 0, 4096);
    port->clb = (unsigned int)(unsigned long)cmd_list;
    port->clbu = 0;

    // Allocate 4KB page for Received FIS (requires 256B alignment)
    void* fis_base = pmm_alloc_page();
    if (!fis_base) return 0;
    memset(fis_base, 0, 4096);
    port->fb = (unsigned int)(unsigned long)fis_base;
    port->fbu = 0;

    // Allocate 4KB page for Command Table (slot 0)
    hba_cmd_tbl_t* cmd_tbl = (hba_cmd_tbl_t*)pmm_alloc_page();
    if (!cmd_tbl) return 0;
    memset(cmd_tbl, 0, 4096);
    cmd_list[0].ctba = (unsigned int)(unsigned long)cmd_tbl;
    cmd_list[0].ctbau = 0;

    // Allocate DMA staging buffer (4KB page)
    void* dma_buf = pmm_alloc_page();
    if (!dma_buf) return 0;
    memset(dma_buf, 0, 4096);

    port_mems[port_idx].cmd_list = cmd_list;
    port_mems[port_idx].fis_base = fis_base;
    port_mems[port_idx].cmd_tbl = cmd_tbl;
    port_mems[port_idx].dma_buffer = dma_buf;

    port_start(port);
    return 1;
}

static int ahci_identify_device(ahci_device_t* dev) {
    hba_port_t* port = dev->port;
    ahci_port_mem_t* mem = &port_mems[dev->port_num];

    port->is = (unsigned int)-1;
    int slot = 0;
    hba_cmd_header_t* cmd_hdr = &mem->cmd_list[slot];
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(unsigned int);
    cmd_hdr->w = 0;
    cmd_hdr->prdtl = 1;
    cmd_hdr->prdbc = 0;

    hba_cmd_tbl_t* cmd_tbl = mem->cmd_tbl;
    memset(cmd_tbl, 0, sizeof(hba_cmd_tbl_t));

    cmd_tbl->prdt_entry[0].dba = (unsigned int)(unsigned long)mem->dma_buffer;
    cmd_tbl->prdt_entry[0].dbau = 0;
    cmd_tbl->prdt_entry[0].dbc = 512 - 1;
    cmd_tbl->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)cmd_tbl->cfis;
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->pmport_c = 0x80;
    cmdfis->command = (dev->type == AHCI_DEV_SATAPI) ? ATA_CMD_IDENTIFY_PACKET : ATA_CMD_IDENTIFY;
    cmdfis->device = 0;

    int timeout = 1000000;
    while ((port->tfd & (0x80 | 0x08)) && --timeout);
    if (timeout <= 0) return -1;

    port->ci = (1U << slot);

    while (1) {
        if ((port->ci & (1U << slot)) == 0) break;
        if (port->is & HBA_PxIS_TFES) return -1;
    }
    if (port->is & HBA_PxIS_TFES) return -1;

    unsigned short* id_buf = (unsigned short*)mem->dma_buffer;

    // Model name is in words 27..46 (40 bytes, byte-swapped)
    int k = 0;
    for (int i = 27; i <= 46; i++) {
        unsigned short w = id_buf[i];
        dev->model[k++] = (char)(w >> 8);
        dev->model[k++] = (char)(w & 0xFF);
    }
    dev->model[40] = '\0';
    for (int i = 39; i >= 0; i--) {
        if (dev->model[i] == ' ' || dev->model[i] == '\0') dev->model[i] = '\0';
        else break;
    }

    // Sectors: words 60-61 for 28-bit LBA, words 100-103 for 48-bit LBA
    unsigned int sectors28 = id_buf[60] | ((unsigned int)id_buf[61] << 16);
    unsigned long long sectors48 = (unsigned long long)id_buf[100] |
                                   ((unsigned long long)id_buf[101] << 16) |
                                   ((unsigned long long)id_buf[102] << 32) |
                                   ((unsigned long long)id_buf[103] << 48);

    if (sectors48 > 0 && sectors48 < 0xFFFFFFFFULL) {
        dev->total_sectors = (unsigned int)sectors48;
    } else {
        dev->total_sectors = sectors28;
    }
    dev->size_mb = (unsigned int)((unsigned long long)dev->total_sectors * 512ULL / (1024ULL * 1024ULL));
    return 0;
}

int ahci_init(void) {
    ahci_device_count = 0;
    ahci_active_drive = -1;
    hba_base = 0;

    debug_puts("\n[AHCI] Scanning PCI bus for SATA AHCI controller...\n");
    pci_device_t* pdev = pci_find_class(PCI_CLASS_MASS_STORAGE, PCI_SUBCLASS_STORAGE_SATA, 0x01);
    if (!pdev) {
        pdev = pci_find_class(PCI_CLASS_MASS_STORAGE, PCI_SUBCLASS_STORAGE_SATA, 0xFF);
    }

    if (!pdev) {
        debug_puts("[AHCI] No PCI SATA AHCI controller found on bus.\n");
        return 0;
    }

    debug_puts("[AHCI] SATA Controller found at PCI ");
    char num[16];
    int_str(pdev->bus, num); debug_puts(num); debug_puts(":");
    int_str(pdev->device, num); debug_puts(num); debug_puts(".");
    int_str(pdev->func, num); debug_puts(num);
    debug_puts(" -> ["); debug_puts(pdev->vendor_name); debug_puts("] ");
    debug_puts(pdev->device_name); debug_puts("\n");

    pci_enable_bus_mastering(pdev);
    pci_enable_memory_space(pdev);

    unsigned int abar_phys = pdev->bar[5];
    if (abar_phys == 0) {
        debug_puts("[AHCI] Error: ABAR (BAR5) is zero!\n");
        return 0;
    }

    hba_base = (hba_mem_t*)(unsigned long)abar_phys;
    debug_puts("[AHCI] ABAR MMIO Base: 0x");
    char hex[16];
    uint_to_str(abar_phys, hex, 16);
    debug_puts(hex);
    debug_puts("\n");

    // Enable AHCI mode in HBA
    hba_base->ghc |= (1U << 31); // Set AE (AHCI Enable)

    unsigned int pi = hba_base->pi;
    debug_puts("[AHCI] Ports Implemented Mask: 0x");
    uint_to_str(pi, hex, 16);
    debug_puts(hex);
    debug_puts("\n");

    for (int i = 0; i < 32; i++) {
        if (pi & (1U << i)) {
            hba_port_t* port = &hba_base->ports[i];
            int type = check_type(port);
            if (type == AHCI_DEV_SATA || type == AHCI_DEV_SATAPI) {
                debug_puts("[AHCI] Port ");
                int_str(i, num); debug_puts(num);
                debug_puts(type == AHCI_DEV_SATA ? ": SATA Drive detected!\n" : ": SATAPI Device detected!\n");

                if (ahci_config_port(i, port)) {
                    ahci_device_t* dev = &ahci_devices[ahci_device_count++];
                    memset(dev, 0, sizeof(ahci_device_t));
                    dev->present = 1;
                    dev->port_num = i;
                    dev->port = port;
                    dev->type = type;

                    if (ahci_identify_device(dev) == 0) {
                        debug_puts("[AHCI] Drive Model: ");
                        debug_puts(dev->model);
                        debug_puts(" | Size: ");
                        int_str(dev->size_mb, num); debug_puts(num);
                        debug_puts(" MB\n");
                    }

                    if (ahci_active_drive < 0 && type == AHCI_DEV_SATA) {
                        ahci_active_drive = ahci_device_count - 1;
                    }
                }
            }
        }
    }

    debug_puts("[AHCI] Initialization complete. Active SATA drives: ");
    int_str(ahci_device_count, num); debug_puts(num); debug_puts("\n\n");
    return (ahci_active_drive >= 0);
}

int ahci_is_available(void) {
    return (ahci_active_drive >= 0 && ahci_devices[ahci_active_drive].present);
}

ahci_device_t* ahci_get_primary_device(void) {
    if (ahci_active_drive >= 0 && ahci_active_drive < ahci_device_count) {
        return &ahci_devices[ahci_active_drive];
    }
    return 0;
}

int ahci_read_sectors(unsigned int lba, int count, unsigned char* buffer) {
    if (!ahci_is_available() || count <= 0 || !buffer) return -1;
    ahci_device_t* dev = &ahci_devices[ahci_active_drive];
    hba_port_t* port = dev->port;
    ahci_port_mem_t* mem = &port_mems[dev->port_num];

    // Read in chunks if count > 8 (staging buffer is 4KB = 8 sectors)
    while (count > 0) {
        int chunk = (count > 8) ? 8 : count;

        port->is = (unsigned int)-1;
        int slot = 0;
        hba_cmd_header_t* cmd_hdr = &mem->cmd_list[slot];
        cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(unsigned int);
        cmd_hdr->w = 0;
        cmd_hdr->prdtl = 1;
        cmd_hdr->prdbc = 0;

        hba_cmd_tbl_t* cmd_tbl = mem->cmd_tbl;
        memset(cmd_tbl, 0, sizeof(hba_cmd_tbl_t));

        cmd_tbl->prdt_entry[0].dba = (unsigned int)(unsigned long)mem->dma_buffer;
        cmd_tbl->prdt_entry[0].dbau = 0;
        cmd_tbl->prdt_entry[0].dbc = (chunk * 512) - 1;
        cmd_tbl->prdt_entry[0].i = 1;

        fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)cmd_tbl->cfis;
        cmdfis->fis_type = FIS_TYPE_REG_H2D;
        cmdfis->pmport_c = 0x80;
        cmdfis->command = ATA_CMD_READ_DMA_EXT;

        cmdfis->lba0 = (unsigned char)(lba & 0xFF);
        cmdfis->lba1 = (unsigned char)((lba >> 8) & 0xFF);
        cmdfis->lba2 = (unsigned char)((lba >> 16) & 0xFF);
        cmdfis->device = 1 << 6;

        cmdfis->lba3 = (unsigned char)((lba >> 24) & 0xFF);
        cmdfis->lba4 = 0;
        cmdfis->lba5 = 0;

        cmdfis->countl = (unsigned char)(chunk & 0xFF);
        cmdfis->counth = (unsigned char)((chunk >> 8) & 0xFF);

        int timeout = 1000000;
        while ((port->tfd & (0x80 | 0x08)) && --timeout);
        if (timeout <= 0) return -1;

        port->ci = (1U << slot);

        while (1) {
            if ((port->ci & (1U << slot)) == 0) break;
            if (port->is & HBA_PxIS_TFES) return -1;
        }
        if (port->is & HBA_PxIS_TFES) return -1;

        memcpy(buffer, mem->dma_buffer, chunk * 512);

        buffer += chunk * 512;
        lba += chunk;
        count -= chunk;
    }
    return 0;
}

int ahci_write_sectors(unsigned int lba, int count, const unsigned char* buffer) {
    if (!ahci_is_available() || count <= 0 || !buffer) return -1;
    ahci_device_t* dev = &ahci_devices[ahci_active_drive];
    hba_port_t* port = dev->port;
    ahci_port_mem_t* mem = &port_mems[dev->port_num];

    while (count > 0) {
        int chunk = (count > 8) ? 8 : count;
        memcpy(mem->dma_buffer, buffer, chunk * 512);

        port->is = (unsigned int)-1;
        int slot = 0;
        hba_cmd_header_t* cmd_hdr = &mem->cmd_list[slot];
        cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(unsigned int);
        cmd_hdr->w = 1;
        cmd_hdr->prdtl = 1;
        cmd_hdr->prdbc = 0;

        hba_cmd_tbl_t* cmd_tbl = mem->cmd_tbl;
        memset(cmd_tbl, 0, sizeof(hba_cmd_tbl_t));

        cmd_tbl->prdt_entry[0].dba = (unsigned int)(unsigned long)mem->dma_buffer;
        cmd_tbl->prdt_entry[0].dbau = 0;
        cmd_tbl->prdt_entry[0].dbc = (chunk * 512) - 1;
        cmd_tbl->prdt_entry[0].i = 1;

        fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)cmd_tbl->cfis;
        cmdfis->fis_type = FIS_TYPE_REG_H2D;
        cmdfis->pmport_c = 0x80;
        cmdfis->command = ATA_CMD_WRITE_DMA_EXT;

        cmdfis->lba0 = (unsigned char)(lba & 0xFF);
        cmdfis->lba1 = (unsigned char)((lba >> 8) & 0xFF);
        cmdfis->lba2 = (unsigned char)((lba >> 16) & 0xFF);
        cmdfis->device = 1 << 6;

        cmdfis->lba3 = (unsigned char)((lba >> 24) & 0xFF);
        cmdfis->lba4 = 0;
        cmdfis->lba5 = 0;

        cmdfis->countl = (unsigned char)(chunk & 0xFF);
        cmdfis->counth = (unsigned char)((chunk >> 8) & 0xFF);

        int timeout = 1000000;
        while ((port->tfd & (0x80 | 0x08)) && --timeout);
        if (timeout <= 0) return -1;

        port->ci = (1U << slot);

        while (1) {
            if ((port->ci & (1U << slot)) == 0) break;
            if (port->is & HBA_PxIS_TFES) return -1;
        }
        if (port->is & HBA_PxIS_TFES) return -1;

        buffer += chunk * 512;
        lba += chunk;
        count -= chunk;
    }
    return 0;
}

int ahci_read_sector(unsigned int lba, unsigned char* buffer) {
    return ahci_read_sectors(lba, 1, buffer);
}

int ahci_write_sector(unsigned int lba, const unsigned char* buffer) {
    return ahci_write_sectors(lba, 1, buffer);
}

#include "pci.h"
#include "debug.h"
#include "string.h"
#include "kernel.h"

static pci_device_t pci_devices[MAX_PCI_DEVICES];
static int pci_device_count = 0;

// Inline assembly port I/O primitives
static inline void pci_io_outl(unsigned short port, unsigned int val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned int pci_io_inl(unsigned short port) {
    unsigned int ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void pci_io_outw(unsigned short port, unsigned short val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned short pci_io_inw(unsigned short port) {
    unsigned short ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void pci_io_outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char pci_io_inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Low-Level Config Space Read/Write
unsigned int pci_read_config32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    return pci_io_inl(PCI_CONFIG_DATA);
}

unsigned short pci_read_config16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    return pci_io_inw(PCI_CONFIG_DATA + (offset & 2));
}

unsigned char pci_read_config8(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    return pci_io_inb(PCI_CONFIG_DATA + (offset & 3));
}

void pci_write_config32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int val) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    pci_io_outl(PCI_CONFIG_DATA, val);
}

void pci_write_config16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned short val) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    pci_io_outw(PCI_CONFIG_DATA + (offset & 2), val);
}

void pci_write_config8(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned char val) {
    unsigned int address = (1U << 31) | ((unsigned int)bus << 16) | ((unsigned int)dev << 11) | ((unsigned int)func << 8) | (offset & 0xFC);
    pci_io_outl(PCI_CONFIG_ADDRESS, address);
    pci_io_outb(PCI_CONFIG_DATA + (offset & 3), val);
}

// BAR Size Calculation
unsigned int pci_get_bar_size(unsigned char bus, unsigned char dev, unsigned char func, unsigned char bar_idx, unsigned int orig_val) {
    if (orig_val == 0) return 0;
    unsigned char reg = PCI_REG_BAR0 + (bar_idx * 4);
    pci_write_config32(bus, dev, func, reg, 0xFFFFFFFF);
    unsigned int read_val = pci_read_config32(bus, dev, func, reg);
    pci_write_config32(bus, dev, func, reg, orig_val); // restore

    if (read_val == 0 || read_val == 0xFFFFFFFF) return 0;

    unsigned int mask;
    if (orig_val & 1) {
        mask = read_val & 0xFFFFFFFC;
    } else {
        mask = read_val & 0xFFFFFFF0;
    }
    return ~mask + 1;
}

// Configuration Control
void pci_enable_bus_mastering(pci_device_t* dev) {
    if (!dev) return;
    unsigned short cmd = pci_read_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND);
    cmd |= PCI_COMMAND_BUS_MASTER;
    pci_write_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND, cmd);
    dev->command = cmd;
}

void pci_enable_memory_space(pci_device_t* dev) {
    if (!dev) return;
    unsigned short cmd = pci_read_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND);
    cmd |= PCI_COMMAND_MEMORY_SPACE;
    pci_write_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND, cmd);
    dev->command = cmd;
}

void pci_enable_io_space(pci_device_t* dev) {
    if (!dev) return;
    unsigned short cmd = pci_read_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND);
    cmd |= PCI_COMMAND_IO_SPACE;
    pci_write_config16(dev->bus, dev->device, dev->func, PCI_REG_COMMAND, cmd);
    dev->command = cmd;
}

// Identification Database
const char* pci_get_vendor_name(unsigned short vendor_id) {
    switch (vendor_id) {
        case 0x8086: return "Intel Corp.";
        case 0x1022: return "AMD Inc.";
        case 0x10DE: return "NVIDIA Corp.";
        case 0x10EC: return "Realtek Semi.";
        case 0x1234: return "QEMU/Bochs";
        case 0x1AF4: return "Red Hat VirtIO";
        case 0x15AD: return "VMware Inc.";
        case 0x80EE: return "VirtualBox";
        case 0x1013: return "Cirrus Logic";
        case 0x5333: return "S3 Graphics";
        case 0x104C: return "Texas Instruments";
        case 0x11AB: return "Marvell Tech.";
        case 0x14E4: return "Broadcom Inc.";
        case 0x1106: return "VIA Tech.";
        default:     return "Unknown Vendor";
    }
}

const char* pci_get_device_name(unsigned short vendor_id, unsigned short device_id) {
    if (vendor_id == 0x8086) {
        switch (device_id) {
            case 0x1237: return "440FX Host PCI Bridge";
            case 0x7000: return "PIIX3 PCI-to-ISA Bridge";
            case 0x7010: return "PIIX3 IDE / ATA Controller";
            case 0x7113: return "PIIX4 ACPI Power Controller";
            case 0x7020: return "PIIX3 USB UHCI Controller";
            case 0x100E: return "82540EM Gigabit Ethernet (e1000)";
            case 0x100F: return "82545EM Gigabit Ethernet";
            case 0x2415: return "82801AA AC'97 Audio Controller";
            case 0x2668: return "ICH6 High Definition Audio";
            case 0x2918: return "ICH9 LPC Interface Controller";
            case 0x2922: return "ICH9 6-Port SATA AHCI Controller";
            case 0x2930: return "ICH9 SMBus Controller";
            case 0x2934: return "ICH9 USB UHCI Controller";
            case 0x293A: return "ICH9 USB2 EHCI Controller";
            case 0x29C0: return "Q35 Host Bridge & Memory Controller";
            default:     return "Intel PCI Device";
        }
    } else if (vendor_id == 0x1234) {
        switch (device_id) {
            case 0x1111: return "QEMU Standard VGA Graphics Adapter";
            default:     return "QEMU Virtual Device";
        }
    } else if (vendor_id == 0x10EC) {
        switch (device_id) {
            case 0x8139: return "RTL8139 Fast Ethernet Controller";
            case 0x8168: return "RTL8168 Gigabit Ethernet";
            default:     return "Realtek Device";
        }
    } else if (vendor_id == 0x1022) {
        switch (device_id) {
            case 0x2000: return "PCnet-PCI II Ethernet Controller";
            default:     return "AMD PCI Device";
        }
    } else if (vendor_id == 0x1AF4) {
        switch (device_id) {
            case 0x1000: return "VirtIO Network Card";
            case 0x1001: return "VirtIO Block Device";
            case 0x1002: return "VirtIO Memory Balloon";
            case 0x1003: return "VirtIO Console";
            case 0x1004: return "VirtIO SCSI Controller";
            case 0x1005: return "VirtIO Entropy RNG";
            case 0x1050: return "VirtIO GPU Graphics";
            default:     return "VirtIO Virtual Device";
        }
    } else if (vendor_id == 0x1013) {
        switch (device_id) {
            case 0x00B8: return "CL-GD5446 VGA Adapter";
            default:     return "Cirrus Logic Device";
        }
    } else if (vendor_id == 0x80EE) {
        switch (device_id) {
            case 0xBEEF: return "VirtualBox Guest Graphics Adapter";
            case 0xCAFE: return "VirtualBox Guest Additions";
            default:     return "VirtualBox Device";
        }
    }
    return "Generic PCI Device";
}

const char* pci_get_class_name(unsigned char class_code, unsigned char subclass, unsigned char prog_if) {
    (void)prog_if;
    switch (class_code) {
        case PCI_CLASS_MASS_STORAGE:
            switch (subclass) {
                case PCI_SUBCLASS_STORAGE_SCSI:   return "Storage: SCSI Controller";
                case PCI_SUBCLASS_STORAGE_IDE:    return "Storage: IDE / ATA Controller";
                case PCI_SUBCLASS_STORAGE_FLOPPY: return "Storage: Floppy Disk";
                case PCI_SUBCLASS_STORAGE_RAID:   return "Storage: RAID Controller";
                case PCI_SUBCLASS_STORAGE_ATA:    return "Storage: ATA (ADMA)";
                case PCI_SUBCLASS_STORAGE_SATA:   return "Storage: SATA (AHCI)";
                case PCI_SUBCLASS_STORAGE_NVME:   return "Storage: NVMe Express";
                default:                          return "Storage Controller";
            }
        case PCI_CLASS_NETWORK:
            switch (subclass) {
                case PCI_SUBCLASS_NET_ETHERNET:   return "Network: Ethernet Controller";
                case PCI_SUBCLASS_NET_TOKEN_RING: return "Network: Token Ring";
                default:                          return "Network Controller";
            }
        case PCI_CLASS_DISPLAY:
            switch (subclass) {
                case PCI_SUBCLASS_DISPLAY_VGA:    return "Display: VGA Controller";
                case PCI_SUBCLASS_DISPLAY_XGA:    return "Display: XGA Controller";
                case PCI_SUBCLASS_DISPLAY_3D:     return "Display: 3D Controller";
                default:                          return "Display Adapter";
            }
        case PCI_CLASS_MULTIMEDIA:
            switch (subclass) {
                case PCI_SUBCLASS_MM_VIDEO:       return "Multimedia: Video Device";
                case PCI_SUBCLASS_MM_AUDIO:       return "Multimedia: Audio (AC'97)";
                case PCI_SUBCLASS_MM_HDA:         return "Multimedia: Intel HDA Audio";
                default:                          return "Multimedia Controller";
            }
        case PCI_CLASS_MEMORY:
            return "Memory Controller";
        case PCI_CLASS_BRIDGE:
            switch (subclass) {
                case PCI_SUBCLASS_BRIDGE_HOST:    return "Bridge: Host Bridge";
                case PCI_SUBCLASS_BRIDGE_ISA:     return "Bridge: PCI-to-ISA Bridge";
                case PCI_SUBCLASS_BRIDGE_PCI:     return "Bridge: PCI-to-PCI Bridge";
                default:                          return "Bridge Controller";
            }
        case PCI_CLASS_SIMPLE_COMM:
            return "Communication Controller";
        case PCI_CLASS_BASE_SYSTEM:
            return "System Peripheral";
        case PCI_CLASS_INPUT_DEVICE:
            return "Input Device Controller";
        case PCI_CLASS_SERIAL_BUS:
            switch (subclass) {
                case PCI_SUBCLASS_SERIAL_FIREWIRE: return "Serial: IEEE 1394 FireWire";
                case PCI_SUBCLASS_SERIAL_USB:      return "Serial: USB Host Controller";
                case PCI_SUBCLASS_SERIAL_SMBUS:    return "Serial: SMBus Controller";
                default:                           return "Serial Bus Controller";
            }
        default:
            return "Other / Unknown Device";
    }
}

// Subsystem Lifecycle & Enumeration
void pci_init(void) {
    pci_device_count = 0;
    debug_puts("\n[PCI] ========================================\n");
    debug_puts("[PCI] Initializing PCI Bus Scanner...\n");

    for (unsigned int bus = 0; bus < 32; bus++) {
        for (unsigned int dev = 0; dev < 32; dev++) {
            unsigned short vendor = pci_read_config16(bus, dev, 0, PCI_REG_VENDOR_ID);
            if (vendor == 0xFFFF || vendor == 0x0000) continue;

            unsigned char header_type = pci_read_config8(bus, dev, 0, PCI_REG_HEADER_TYPE);
            int max_funcs = (header_type & 0x80) ? 8 : 1;

            for (unsigned int func = 0; func < (unsigned int)max_funcs; func++) {
                vendor = pci_read_config16(bus, dev, func, PCI_REG_VENDOR_ID);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;

                if (pci_device_count >= MAX_PCI_DEVICES) break;

                pci_device_t* pdev = &pci_devices[pci_device_count++];
                memset(pdev, 0, sizeof(pci_device_t));

                pdev->bus = bus;
                pdev->device = dev;
                pdev->func = func;
                pdev->vendor_id = vendor;
                pdev->device_id = pci_read_config16(bus, dev, func, PCI_REG_DEVICE_ID);
                pdev->command = pci_read_config16(bus, dev, func, PCI_REG_COMMAND);
                pdev->status = pci_read_config16(bus, dev, func, PCI_REG_STATUS);
                pdev->revision_id = pci_read_config8(bus, dev, func, PCI_REG_REVISION_ID);
                pdev->prog_if = pci_read_config8(bus, dev, func, PCI_REG_PROG_IF);
                pdev->subclass = pci_read_config8(bus, dev, func, PCI_REG_SUBCLASS);
                pdev->class_code = pci_read_config8(bus, dev, func, PCI_REG_CLASS_CODE);
                pdev->header_type = pci_read_config8(bus, dev, func, PCI_REG_HEADER_TYPE);
                pdev->irq = pci_read_config8(bus, dev, func, PCI_REG_INTERRUPT_LINE);

                // Read BARs for standard device headers (Type 0)
                if ((pdev->header_type & 0x7F) == 0x00) {
                    for (int b = 0; b < 6; b++) {
                        unsigned int bar_raw = pci_read_config32(bus, dev, func, PCI_REG_BAR0 + (b * 4));
                        pdev->bar_is_io[b] = (bar_raw & 1) ? 1 : 0;
                        pdev->bar[b] = pdev->bar_is_io[b] ? (bar_raw & ~0x3) : (bar_raw & ~0xF);
                        pdev->bar_size[b] = pci_get_bar_size(bus, dev, func, b, bar_raw);
                    }
                }

                strncpy(pdev->vendor_name, pci_get_vendor_name(pdev->vendor_id), sizeof(pdev->vendor_name) - 1);
                pdev->vendor_name[sizeof(pdev->vendor_name) - 1] = '\0';

                strncpy(pdev->device_name, pci_get_device_name(pdev->vendor_id, pdev->device_id), sizeof(pdev->device_name) - 1);
                pdev->device_name[sizeof(pdev->device_name) - 1] = '\0';

                strncpy(pdev->class_name, pci_get_class_name(pdev->class_code, pdev->subclass, pdev->prog_if), sizeof(pdev->class_name) - 1);
                pdev->class_name[sizeof(pdev->class_name) - 1] = '\0';

                // Serial Debug Output
                debug_puts("[PCI] ");
                char bdf[32];
                int_str(bus, bdf);
                debug_puts(bdf);
                debug_puts(":");
                int_str(dev, bdf);
                debug_puts(bdf);
                debug_puts(".");
                int_str(func, bdf);
                debug_puts(bdf);
                debug_puts(" -> [");
                debug_puts(pdev->vendor_name);
                debug_puts("] ");
                debug_puts(pdev->device_name);
                debug_puts(" | ");
                debug_puts(pdev->class_name);
                debug_puts("\n");
            }
        }
    }

    debug_puts("[PCI] Scanner completed. Total detected devices: ");
    char num[16];
    int_str(pci_device_count, num);
    debug_puts(num);
    debug_puts("\n[PCI] ========================================\n\n");
}

int pci_get_device_count(void) {
    return pci_device_count;
}

pci_device_t* pci_get_device(int index) {
    if (index >= 0 && index < pci_device_count) {
        return &pci_devices[index];
    }
    return 0;
}

pci_device_t* pci_find_device(unsigned short vendor_id, unsigned short device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id && pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return 0;
}

pci_device_t* pci_find_class(unsigned char class_code, unsigned char subclass, unsigned char prog_if) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].class_code == class_code &&
            pci_devices[i].subclass == subclass &&
            (prog_if == 0xFF || pci_devices[i].prog_if == prog_if)) {
            return &pci_devices[i];
        }
    }
    return 0;
}

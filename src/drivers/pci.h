#ifndef PCI_H
#define PCI_H

// PCI Configuration Mechanism #1 Ports
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// Standard PCI Configuration Register Offsets
#define PCI_REG_VENDOR_ID       0x00
#define PCI_REG_DEVICE_ID       0x02
#define PCI_REG_COMMAND         0x04
#define PCI_REG_STATUS          0x06
#define PCI_REG_REVISION_ID     0x08
#define PCI_REG_PROG_IF         0x09
#define PCI_REG_SUBCLASS        0x0A
#define PCI_REG_CLASS_CODE      0x0B
#define PCI_REG_CACHE_LINE_SIZE 0x0C
#define PCI_REG_LATENCY_TIMER   0x0D
#define PCI_REG_HEADER_TYPE     0x0E
#define PCI_REG_BIST            0x0F
#define PCI_REG_BAR0            0x10
#define PCI_REG_BAR1            0x14
#define PCI_REG_BAR2            0x18
#define PCI_REG_BAR3            0x1C
#define PCI_REG_BAR4            0x20
#define PCI_REG_BAR5            0x24
#define PCI_REG_CARDBUS_CIS     0x28
#define PCI_REG_SUBSYS_VENDOR   0x2C
#define PCI_REG_SUBSYS_ID       0x2E
#define PCI_REG_EXP_ROM_BASE    0x30
#define PCI_REG_CAP_PTR         0x34
#define PCI_REG_INTERRUPT_LINE  0x3C
#define PCI_REG_INTERRUPT_PIN   0x3D
#define PCI_REG_MIN_GNT         0x3E
#define PCI_REG_MAX_LAT         0x3F

// PCI Command Register Bit Flags
#define PCI_COMMAND_IO_SPACE           (1 << 0)
#define PCI_COMMAND_MEMORY_SPACE       (1 << 1)
#define PCI_COMMAND_BUS_MASTER         (1 << 2)
#define PCI_COMMAND_SPECIAL_CYCLES     (1 << 3)
#define PCI_COMMAND_MEM_WRITE_INVALID  (1 << 4)
#define PCI_COMMAND_VGA_PALETTE_SNOOP  (1 << 5)
#define PCI_COMMAND_PARITY_ERROR_RESP  (1 << 6)
#define PCI_COMMAND_SERR_ENABLE        (1 << 8)
#define PCI_COMMAND_FAST_B2B           (1 << 9)
#define PCI_COMMAND_INTERRUPT_DISABLE  (1 << 10)

// Major PCI Base Class Codes
#define PCI_CLASS_UNCLASSIFIED         0x00
#define PCI_CLASS_MASS_STORAGE         0x01
#define PCI_CLASS_NETWORK              0x02
#define PCI_CLASS_DISPLAY              0x03
#define PCI_CLASS_MULTIMEDIA           0x04
#define PCI_CLASS_MEMORY               0x05
#define PCI_CLASS_BRIDGE               0x06
#define PCI_CLASS_SIMPLE_COMM          0x07
#define PCI_CLASS_BASE_SYSTEM          0x08
#define PCI_CLASS_INPUT_DEVICE         0x09
#define PCI_CLASS_DOCKING_STATION      0x0A
#define PCI_CLASS_PROCESSOR            0x0B
#define PCI_CLASS_SERIAL_BUS           0x0C
#define PCI_CLASS_WIRELESS             0x0D
#define PCI_CLASS_INTELLIGENT_IO       0x0E
#define PCI_CLASS_SATELLITE            0x0F
#define PCI_CLASS_ENCRYPTION           0x10
#define PCI_CLASS_SIGNAL_PROCESSING    0x11

// Subclasses for Mass Storage (0x01)
#define PCI_SUBCLASS_STORAGE_SCSI      0x00
#define PCI_SUBCLASS_STORAGE_IDE       0x01
#define PCI_SUBCLASS_STORAGE_FLOPPY    0x02
#define PCI_SUBCLASS_STORAGE_IPI       0x03
#define PCI_SUBCLASS_STORAGE_RAID      0x04
#define PCI_SUBCLASS_STORAGE_ATA       0x05
#define PCI_SUBCLASS_STORAGE_SATA      0x06
#define PCI_SUBCLASS_STORAGE_SAS       0x07
#define PCI_SUBCLASS_STORAGE_NVME      0x08
#define PCI_SUBCLASS_STORAGE_OTHER     0x80

// Subclasses for Network (0x02)
#define PCI_SUBCLASS_NET_ETHERNET      0x00
#define PCI_SUBCLASS_NET_TOKEN_RING    0x01
#define PCI_SUBCLASS_NET_FDDI          0x02
#define PCI_SUBCLASS_NET_ATM           0x03
#define PCI_SUBCLASS_NET_ISDN          0x04
#define PCI_SUBCLASS_NET_OTHER         0x80

// Subclasses for Display (0x03)
#define PCI_SUBCLASS_DISPLAY_VGA       0x00
#define PCI_SUBCLASS_DISPLAY_XGA       0x01
#define PCI_SUBCLASS_DISPLAY_3D        0x02
#define PCI_SUBCLASS_DISPLAY_OTHER     0x80

// Subclasses for Multimedia (0x04)
#define PCI_SUBCLASS_MM_VIDEO          0x00
#define PCI_SUBCLASS_MM_AUDIO          0x01 // AC'97, etc.
#define PCI_SUBCLASS_MM_TELEPHONY      0x02
#define PCI_SUBCLASS_MM_HDA            0x03 // Intel High Definition Audio
#define PCI_SUBCLASS_MM_OTHER          0x80

// Subclasses for Bridge (0x06)
#define PCI_SUBCLASS_BRIDGE_HOST       0x00
#define PCI_SUBCLASS_BRIDGE_ISA        0x01
#define PCI_SUBCLASS_BRIDGE_EISA       0x02
#define PCI_SUBCLASS_BRIDGE_MCA        0x03
#define PCI_SUBCLASS_BRIDGE_PCI        0x04
#define PCI_SUBCLASS_BRIDGE_PCMCIA     0x05
#define PCI_SUBCLASS_BRIDGE_NUBUS      0x06
#define PCI_SUBCLASS_BRIDGE_CARDBUS    0x07
#define PCI_SUBCLASS_BRIDGE_OTHER      0x80

// Subclasses for Serial Bus (0x0C)
#define PCI_SUBCLASS_SERIAL_FIREWIRE   0x00
#define PCI_SUBCLASS_SERIAL_ACCESS     0x01
#define PCI_SUBCLASS_SERIAL_SSA        0x02
#define PCI_SUBCLASS_SERIAL_USB        0x03
#define PCI_SUBCLASS_SERIAL_FIBRE      0x04
#define PCI_SUBCLASS_SERIAL_SMBUS      0x05

#define MAX_PCI_DEVICES 64

// Representation of a detected PCI device
typedef struct {
    unsigned char bus;
    unsigned char device;
    unsigned char func;
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned short command;
    unsigned short status;
    unsigned char revision_id;
    unsigned char prog_if;
    unsigned char subclass;
    unsigned char class_code;
    unsigned char header_type;
    unsigned char irq;
    unsigned int bar[6];
    unsigned int bar_size[6];
    unsigned char bar_is_io[6];
    char vendor_name[32];
    char device_name[48];
    char class_name[36];
} pci_device_t;

// Low-Level Config Space Read/Write
unsigned int   pci_read_config32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);
unsigned short pci_read_config16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);
unsigned char  pci_read_config8(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);

void pci_write_config32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int val);
void pci_write_config16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned short val);
void pci_write_config8(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned char val);

// Subsystem Lifecycle & Enumeration
void pci_init(void);
int  pci_get_device_count(void);
pci_device_t* pci_get_device(int index);
pci_device_t* pci_find_device(unsigned short vendor_id, unsigned short device_id);
pci_device_t* pci_find_class(unsigned char class_code, unsigned char subclass, unsigned char prog_if);

// Device Configuration Helpers
void pci_enable_bus_mastering(pci_device_t* dev);
void pci_enable_memory_space(pci_device_t* dev);
void pci_enable_io_space(pci_device_t* dev);
unsigned int pci_get_bar_size(unsigned char bus, unsigned char dev, unsigned char func, unsigned char bar_idx, unsigned int orig_val);

// Human-Readable Name Lookups
const char* pci_get_vendor_name(unsigned short vendor_id);
const char* pci_get_device_name(unsigned short vendor_id, unsigned short device_id);
const char* pci_get_class_name(unsigned char class_code, unsigned char subclass, unsigned char prog_if);

#endif // PCI_H

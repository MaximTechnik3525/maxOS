unsigned int inl(unsigned short port);
void outl(unsigned short port, unsigned int data);
void print_string(char* str, int x, int y, unsigned short color);
void int_str(int num, char* str);
extern win_y;
unsigned int pci_read_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int address;
    unsigned int lbus = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));
    outl(0x0CF8, address);
    return inl(0x0CFC);
}
void pci_scan(int txt_x, int txt_y) {
    int dev_count = 0;
    char ven_str[10] = {0};
    char dev_str[10] = {0};
    for (int bus = 0; bus < 8; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                unsigned int data = pci_read_config(bus, slot, func, 0);
                if ((data & 0xFFFF) != 0xFFFF) {
                    unsigned short vendorId = data & 0xFFFF;
                    unsigned short deviceId = (data >> 16) & 0xFFFF;
                    dev_count++;
                    int_str(vendorId, ven_str);
                    int_str(dev_count, dev_str);
                    print_string("PCI Device found! Vendor ID:", txt_x, txt_y, 0x0000);
                    print_string(ven_str, txt_x + 260, txt_y, 0x0000);
                    if (txt_y < win_y + 400) { txt_y += 15; }
                }
            }
        }
    }
    print_string("Total devices:", txt_x, txt_y, 0x0000);
    print_string(dev_str, txt_x + 135, txt_y, 0x0000);
}
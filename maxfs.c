#include "maxfs.h"
#include "ata.h"

struct VirtualFile ram_disk[MAXFS_MAX_FILES];
int maxfs_disk_mounted = 0;
static char current_volume_label[32] = "maxOS Disk";

static struct DiskInode inode_cache[MAXFS_MAX_FILES];
static unsigned char bitmap_cache[MAXFS_BITMAP_SECTORS * 512];

static void sync_ram_disk(void) {
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (inode_cache[i].flags & 0x01) {
            ram_disk[i].exists = 1;
            ram_disk[i].size = inode_cache[i].size;
            int n = 0;
            while (inode_cache[i].name[n] != '\0' && n < (MAXFS_NAME_LEN - 1)) {
                ram_disk[i].name[n] = inode_cache[i].name[n];
                n++;
            }
            ram_disk[i].name[n] = '\0';

            // Read content from disk if disk mounted
            if (maxfs_disk_mounted && inode_cache[i].start_lba >= MAXFS_DATA_LBA) {
                unsigned char sbuf[512];
                unsigned int read_bytes = 0;
                for (unsigned int s = 0; s < inode_cache[i].sectors && read_bytes < (MAXFS_CONTENT_LEN - 1); s++) {
                    if (ata_read_sector(inode_cache[i].start_lba + s, sbuf) == 0) {
                        for (int b = 0; b < 512 && read_bytes < (MAXFS_CONTENT_LEN - 1) && read_bytes < inode_cache[i].size; b++) {
                            ram_disk[i].content[read_bytes++] = (char)sbuf[b];
                        }
                    }
                }
                ram_disk[i].content[read_bytes] = '\0';
            }
        } else {
            ram_disk[i].exists = 0;
            ram_disk[i].size = 0;
            ram_disk[i].name[0] = '\0';
            ram_disk[i].content[0] = '\0';
        }
    }
}

int maxfs_mount(void) {
    if (!ata_is_available()) {
        maxfs_disk_mounted = 0;
        return -1;
    }

    unsigned char sbuf[512];
    if (ata_read_sector(MAXFS_SUPERBLOCK_LBA, sbuf) != 0) {
        maxfs_disk_mounted = 0;
        return -1;
    }

    struct DiskSuperblock* sb = (struct DiskSuperblock*)sbuf;
    if (sb->magic != MAXFS_MAGIC) {
        maxfs_disk_mounted = 0;
        return -1;
    }

    int v = 0;
    while (sb->volume_name[v] != '\0' && v < 31) {
        current_volume_label[v] = sb->volume_name[v];
        v++;
    }
    current_volume_label[v] = '\0';

    // Read Inode table
    for (int s = 0; s < MAXFS_INODE_SECTORS; s++) {
        if (ata_read_sector(MAXFS_INODE_LBA + s, (unsigned char*)&inode_cache[s * 8]) != 0) {
            maxfs_disk_mounted = 0;
            return -1;
        }
    }

    // Read Bitmap
    for (int s = 0; s < MAXFS_BITMAP_SECTORS; s++) {
        if (ata_read_sector(MAXFS_BITMAP_LBA + s, &bitmap_cache[s * 512]) != 0) {
            maxfs_disk_mounted = 0;
            return -1;
        }
    }

    maxfs_disk_mounted = 1;
    sync_ram_disk();
    return 0;
}

int maxfs_format(const char* volume_label) {
    if (!ata_is_available()) {
        // Fallback: format RAM disk only
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            ram_disk[i].exists = 0;
            ram_disk[i].size = 0;
            ram_disk[i].name[0] = '\0';
            ram_disk[i].content[0] = '\0';
        }
        return 0;
    }

    // Prepare Superblock
    struct DiskSuperblock sb;
    for (int i = 0; i < 512; i++) ((unsigned char*)&sb)[i] = 0;
    sb.magic = MAXFS_MAGIC;
    sb.version = MAXFS_VERSION;
    sb.block_size = 512;
    sb.total_sectors = ata_primary_master.total_sectors;
    sb.inode_count = MAXFS_MAX_FILES;
    sb.inode_table_lba = MAXFS_INODE_LBA;
    sb.bitmap_lba = MAXFS_BITMAP_LBA;
    sb.bitmap_sectors = MAXFS_BITMAP_SECTORS;
    sb.data_start_lba = MAXFS_DATA_LBA;
    sb.free_sectors = ata_primary_master.total_sectors - MAXFS_DATA_LBA;
    sb.free_inodes = MAXFS_MAX_FILES;

    int p = 0;
    while (volume_label[p] != '\0' && p < 31) {
        sb.volume_name[p] = volume_label[p];
        current_volume_label[p] = volume_label[p];
        p++;
    }
    sb.volume_name[p] = '\0';
    current_volume_label[p] = '\0';

    // Write Superblock to LBA 1
    if (ata_write_sector(MAXFS_SUPERBLOCK_LBA, (const unsigned char*)&sb) != 0) {
        return -1;
    }

    // Clear Inode table
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        for (int b = 0; b < 32; b++) inode_cache[i].name[b] = '\0';
        inode_cache[i].size = 0;
        inode_cache[i].start_lba = 0;
        inode_cache[i].sectors = 0;
        inode_cache[i].flags = 0;
        inode_cache[i].created = 0;
    }
    for (int s = 0; s < MAXFS_INODE_SECTORS; s++) {
        if (ata_write_sector(MAXFS_INODE_LBA + s, (const unsigned char*)&inode_cache[s * 8]) != 0) {
            return -1;
        }
    }

    // Clear Bitmap: mark first 256 sectors as reserved for system (used = 1)
    for (int i = 0; i < MAXFS_BITMAP_SECTORS * 512; i++) {
        bitmap_cache[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        bitmap_cache[i] = 0xFF; // sectors 0..255 used (MBR, maxFS meta, Kernel)
    }
    for (int s = 0; s < MAXFS_BITMAP_SECTORS; s++) {
        if (ata_write_sector(MAXFS_BITMAP_LBA + s, &bitmap_cache[s * 512]) != 0) {
            return -1;
        }
    }

    ata_flush();
    maxfs_disk_mounted = 1;
    sync_ram_disk();
    return 0;
}

void maxfs_init(void) {
    ata_init();
    if (maxfs_mount() != 0) {
        // Disk not formatted yet or not available; initialize empty in-memory structure
        maxfs_disk_mounted = 0;
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            ram_disk[i].exists = 0;
            ram_disk[i].size = 0;
            ram_disk[i].name[0] = '\0';
            ram_disk[i].content[0] = '\0';
            inode_cache[i].flags = 0;
        }
    }
}

static int alloc_sectors(unsigned int count) {
    unsigned int total_bits = MAXFS_BITMAP_SECTORS * 512 * 8;
    for (unsigned int bit = MAXFS_DATA_LBA; bit + count <= total_bits; bit++) {
        int fit = 1;
        for (unsigned int k = 0; k < count; k++) {
            unsigned int idx = bit + k;
            if (bitmap_cache[idx / 8] & (1 << (idx % 8))) {
                fit = 0;
                bit = idx; // skip forward
                break;
            }
        }
        if (fit) {
            for (unsigned int k = 0; k < count; k++) {
                unsigned int idx = bit + k;
                bitmap_cache[idx / 8] |= (1 << (idx % 8));
            }
            return bit;
        }
    }
    return -1;
}

static void free_sectors(unsigned int start_lba, unsigned int count) {
    if (start_lba < MAXFS_DATA_LBA) return;
    for (unsigned int k = 0; k < count; k++) {
        unsigned int idx = start_lba + k;
        bitmap_cache[idx / 8] &= ~(1 << (idx % 8));
    }
}

int maxfs_write_file(const char* name, const char* content, unsigned int len) {
    if (len > (MAXFS_CONTENT_LEN - 1)) len = MAXFS_CONTENT_LEN - 1;

    // Check if file already exists
    int slot = -1;
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (inode_cache[i].flags & 0x01) {
            int match = 1, p = 0;
            while (name[p] != '\0' || inode_cache[i].name[p] != '\0') {
                if (name[p] != inode_cache[i].name[p]) { match = 0; break; }
                p++;
            }
            if (match) {
                slot = i;
                break;
            }
        }
    }

    unsigned int needed_sectors = (len + 511) / 512;
    if (needed_sectors == 0) needed_sectors = 1;

    if (slot != -1) {
        // Update existing file
        if (maxfs_disk_mounted) {
            if (needed_sectors > inode_cache[slot].sectors) {
                free_sectors(inode_cache[slot].start_lba, inode_cache[slot].sectors);
                int new_lba = alloc_sectors(needed_sectors);
                if (new_lba == -1) return -1;
                inode_cache[slot].start_lba = new_lba;
            }
            inode_cache[slot].sectors = needed_sectors;
            inode_cache[slot].size = len;

            // Write content
            unsigned char sbuf[512];
            unsigned int written = 0;
            for (unsigned int s = 0; s < needed_sectors; s++) {
                for (int b = 0; b < 512; b++) {
                    sbuf[b] = (written < len) ? (unsigned char)content[written++] : 0;
                }
                ata_write_sector(inode_cache[slot].start_lba + s, sbuf);
            }

            // Write updated inode sector
            int sec = slot / 8;
            ata_write_sector(MAXFS_INODE_LBA + sec, (const unsigned char*)&inode_cache[sec * 8]);
            // Write bitmap
            for (int s = 0; s < MAXFS_BITMAP_SECTORS; s++) {
                ata_write_sector(MAXFS_BITMAP_LBA + s, &bitmap_cache[s * 512]);
            }
            ata_flush();
        }
    } else {
        // Find free inode slot
        for (int i = 0; i < MAXFS_MAX_FILES; i++) {
            if (!(inode_cache[i].flags & 0x01)) {
                slot = i;
                break;
            }
        }
        if (slot == -1) return -1; // No free inode

        int n = 0;
        while (name[n] != '\0' && n < 31) {
            inode_cache[slot].name[n] = name[n];
            n++;
        }
        inode_cache[slot].name[n] = '\0';
        inode_cache[slot].size = len;
        inode_cache[slot].flags = 0x01; // USED

        if (maxfs_disk_mounted) {
            int lba = alloc_sectors(needed_sectors);
            if (lba == -1) {
                inode_cache[slot].flags = 0;
                return -1;
            }
            inode_cache[slot].start_lba = lba;
            inode_cache[slot].sectors = needed_sectors;

            // Write content sectors
            unsigned char sbuf[512];
            unsigned int written = 0;
            for (unsigned int s = 0; s < needed_sectors; s++) {
                for (int b = 0; b < 512; b++) {
                    sbuf[b] = (written < len) ? (unsigned char)content[written++] : 0;
                }
                ata_write_sector(inode_cache[slot].start_lba + s, sbuf);
            }

            // Write inode table sector
            int sec = slot / 8;
            ata_write_sector(MAXFS_INODE_LBA + sec, (const unsigned char*)&inode_cache[sec * 8]);
            // Write bitmap sectors
            for (int s = 0; s < MAXFS_BITMAP_SECTORS; s++) {
                ata_write_sector(MAXFS_BITMAP_LBA + s, &bitmap_cache[s * 512]);
            }
            ata_flush();
        } else {
            inode_cache[slot].start_lba = 0;
            inode_cache[slot].sectors = needed_sectors;
        }
    }

    sync_ram_disk();
    // Copy content directly into ram_disk cache
    for (unsigned int i = 0; i < len; i++) {
        ram_disk[slot].content[i] = content[i];
    }
    ram_disk[slot].content[len] = '\0';
    ram_disk[slot].size = len;
    ram_disk[slot].exists = 1;

    return slot;
}

int maxfs_create_file(char* name, char* text) {
    int len = 0;
    while (text[len] != '\0') len++;
    return maxfs_write_file(name, text, len);
}

int create_file(char* name, char* text) {
    return maxfs_create_file(name, text);
}

int maxfs_read_file(const char* name, char* buffer, unsigned int max_len) {
    int slot = maxfs_find_file(name);
    if (slot == -1) return -1;

    unsigned int to_read = ram_disk[slot].size;
    if (to_read > (max_len - 1)) to_read = max_len - 1;

    for (unsigned int i = 0; i < to_read; i++) {
        buffer[i] = ram_disk[slot].content[i];
    }
    buffer[to_read] = '\0';
    return to_read;
}

int maxfs_delete_file(const char* name) {
    int slot = maxfs_find_file(name);
    if (slot == -1) return -1;

    if (maxfs_disk_mounted && inode_cache[slot].start_lba >= MAXFS_DATA_LBA) {
        free_sectors(inode_cache[slot].start_lba, inode_cache[slot].sectors);
        for (int s = 0; s < MAXFS_BITMAP_SECTORS; s++) {
            ata_write_sector(MAXFS_BITMAP_LBA + s, &bitmap_cache[s * 512]);
        }
    }

    inode_cache[slot].flags = 0;
    inode_cache[slot].name[0] = '\0';
    inode_cache[slot].size = 0;
    inode_cache[slot].start_lba = 0;
    inode_cache[slot].sectors = 0;

    if (maxfs_disk_mounted) {
        int sec = slot / 8;
        ata_write_sector(MAXFS_INODE_LBA + sec, (const unsigned char*)&inode_cache[sec * 8]);
        ata_flush();
    }

    sync_ram_disk();
    return 0;
}

int maxfs_find_file(const char* name) {
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (ram_disk[i].exists) {
            int match = 1, p = 0;
            while (name[p] != '\0' || ram_disk[i].name[p] != '\0') {
                if (name[p] != ram_disk[i].name[p]) { match = 0; break; }
                p++;
            }
            if (match) return i;
        }
    }
    return -1;
}

struct VirtualFile* maxfs_get_file(int index) {
    if (index >= 0 && index < MAXFS_MAX_FILES && ram_disk[index].exists) {
        return &ram_disk[index];
    }
    return 0;
}

int maxfs_is_mounted(void) {
    return maxfs_disk_mounted;
}

const char* maxfs_get_volume_label(void) {
    return current_volume_label;
}

struct DiskInode* maxfs_get_inode(int index) {
    if (index >= 0 && index < MAXFS_MAX_FILES && (inode_cache[index].flags & 0x01)) {
        return &inode_cache[index];
    }
    return 0;
}

#ifndef MAXFS_H
#define MAXFS_H

#include "ata.h"

#define MAXFS_MAGIC 0x5346584D /* "MXFS" */
#define MAXFS_VERSION 2

#define MAXFS_MAX_FILES 32
#define MAXFS_NAME_LEN 32
#define MAXFS_CONTENT_LEN 16384

#define MAXFS_SUPERBLOCK_LBA 1
#define MAXFS_INODE_LBA 2
#define MAXFS_INODE_SECTORS 4
#define MAXFS_BITMAP_LBA 6
#define MAXFS_BITMAP_SECTORS 2
#define MAXFS_DATA_LBA 256

#pragma pack(push, 1)
struct DiskInode {
    char name[32];
    unsigned int size;
    unsigned int start_lba;
    unsigned int sectors;
    unsigned short flags; // 1 = USED, 2 = SYSTEM
    unsigned short created;
    unsigned char reserved[16];
};

struct DiskSuperblock {
    unsigned int magic;
    unsigned short version;
    unsigned short block_size;
    unsigned int total_sectors;
    unsigned int inode_count;
    unsigned int inode_table_lba;
    unsigned int bitmap_lba;
    unsigned int bitmap_sectors;
    unsigned int data_start_lba;
    unsigned int free_sectors;
    unsigned int free_inodes;
    char volume_name[32];
    unsigned char reserved[440];
};
#pragma pack(pop)

// Legacy compatibility struct for Explorer & Shell
struct VirtualFile {
    char name[MAXFS_NAME_LEN];
    char content[MAXFS_CONTENT_LEN];
    int size;
    int exists;
};

extern struct VirtualFile ram_disk[MAXFS_MAX_FILES];
extern int maxfs_disk_mounted;

void maxfs_init(void);
int maxfs_format(const char* volume_label);
int maxfs_mount(void);
int maxfs_create_file(char* name, char* text);
int maxfs_write_file(const char* name, const char* content, unsigned int len);
int maxfs_read_file(const char* name, char* buffer, unsigned int max_len);
int maxfs_read_binary(const char* name, void* buffer, unsigned int max_len);
int maxfs_write_binary(const char* name, const void* buffer, unsigned int len);
int maxfs_delete_file(const char* name);
int maxfs_is_mounted(void);
const char* maxfs_get_volume_label(void);
struct DiskInode* maxfs_get_inode(int index);

// Legacy API wrappers
int create_file(char* name, char* text);
struct VirtualFile* maxfs_get_file(int index);
int maxfs_find_file(const char* name);

#endif // MAXFS_H

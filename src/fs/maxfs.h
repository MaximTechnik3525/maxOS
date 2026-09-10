#ifndef MAXFS_H
#define MAXFS_H

#include "ata.h"

#define MAXFS_MAGIC 0x5346584D /* "MXFS" */
#define MAXFS_VERSION 3

#define MAXFS_MAX_FILES 1024
#define MAXFS_NAME_LEN 32
#define MAXFS_CONTENT_LEN 16384
#define MAXFS_SUPERBLOCK_LBA 1
#define MAXFS_INODE_LBA 1024
#define MAXFS_INODE_SECTORS 128
#define MAXFS_BITMAP_LBA 1152
#define MAXFS_BITMAP_SECTORS 32
#define MAXFS_DATA_LBA 1200

#define MAXFS_FLAG_USED   0x01
#define MAXFS_FLAG_SYSTEM 0x02
#define MAXFS_FLAG_DIR    0x04
#define MAXFS_ROOT_INODE  0

#pragma pack(push, 1)
struct DiskInode {
    char name[32];
    unsigned int size;
    unsigned int start_lba;
    unsigned int sectors;
    unsigned short flags; // 0x01 = USED, 0x02 = SYSTEM, 0x04 = DIRECTORY
    unsigned short created;
    unsigned int parent_inode; // 0 = root (/), or inode index of parent directory
    unsigned char reserved[12];
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

// In-memory file structure for Explorer, shell & apps
struct VirtualFile {
    char name[MAXFS_NAME_LEN];
    char content[MAXFS_CONTENT_LEN];
    int size;
    int exists;
    int is_dir;
    unsigned int parent_inode;
};

extern struct VirtualFile ram_disk[MAXFS_MAX_FILES];
extern int maxfs_disk_mounted;

void maxfs_init(void);
int maxfs_format(const char* volume_label);
int maxfs_mount(void);
int maxfs_create_file(char* name, char* text);
int maxfs_write_file(const char* name, const char* content, unsigned int len);
int maxfs_write_file_in(unsigned int parent_inode, const char* name, const char* content, unsigned int len);
int maxfs_read_file(const char* name, char* buffer, unsigned int max_len);
int maxfs_read_binary(const char* name, void* buffer, unsigned int max_len);
int maxfs_write_binary(const char* name, const void* buffer, unsigned int len);
int maxfs_delete_file(const char* name);
int maxfs_delete_inode(int index);
int maxfs_rename(int index, const char* new_name);
int maxfs_is_mounted(void);
const char* maxfs_get_volume_label(void);
struct DiskInode* maxfs_get_inode(int index);

// Directory operations
int maxfs_mkdir(const char* path);
int maxfs_create_dir(const char* name, unsigned int parent_inode);
int maxfs_rmdir(const char* path);
int maxfs_is_dir(int index);
unsigned int maxfs_get_parent(int index);
int maxfs_list_dir(unsigned int dir_inode, int* out_indices, int max_entries);
int maxfs_resolve_path(const char* path, unsigned int* out_parent, char* out_name);
int maxfs_find_in_dir(unsigned int dir_inode, const char* name);
void maxfs_get_dir_path(unsigned int dir_inode, char* out_buf, int max_len);

// Legacy API wrappers
int create_file(char* name, char* text);
struct VirtualFile* maxfs_get_file(int index);
int maxfs_find_file(const char* name);

#endif // MAXFS_H

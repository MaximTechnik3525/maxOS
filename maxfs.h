#ifndef MAXFS_H
#define MAXFS_H

#define MAXFS_MAX_FILES   5
#define MAXFS_NAME_LEN    12
#define MAXFS_CONTENT_LEN 100

struct VirtualFile {
    char name[MAXFS_NAME_LEN];
    int size;
    char content[MAXFS_CONTENT_LEN];
    int exists;
};

extern struct VirtualFile ram_disk[MAXFS_MAX_FILES];

void maxfs_init(void);
void maxfs_format(void);
int create_file(char* name, char* text);
int maxfs_create_file(char* name, char* text);
struct VirtualFile* maxfs_get_file(int index);
int maxfs_find_file(const char* name);

#endif // MAXFS_H

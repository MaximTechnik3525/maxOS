#include "maxfs.h"

struct VirtualFile ram_disk[MAXFS_MAX_FILES];

void maxfs_format(void) {
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        ram_disk[i].exists = 0;
        ram_disk[i].size = 0;
        for (int n = 0; n < MAXFS_NAME_LEN; n++) {
            ram_disk[i].name[n] = '\0';
        }
        for (int t = 0; t < MAXFS_CONTENT_LEN; t++) {
            ram_disk[i].content[t] = '\0';
        }
    }
}

void maxfs_init(void) {
    maxfs_format();
}

int create_file(char* name, char* text) {
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (ram_disk[i].exists == 0) {
            int n = 0;
            while (name[n] != '\0' && n < (MAXFS_NAME_LEN - 1)) {
                ram_disk[i].name[n] = name[n];
                n++;
            }
            ram_disk[i].name[n] = '\0';
            int t = 0;
            while (text[t] != '\0' && t < (MAXFS_CONTENT_LEN - 1)) {
                ram_disk[i].content[t] = text[t];
                t++;
            }
            ram_disk[i].content[t] = '\0';
            ram_disk[i].size = t;
            ram_disk[i].exists = 1;
            return i;
        }
    }
    return -1;
}

int maxfs_create_file(char* name, char* text) {
    return create_file(name, text);
}

struct VirtualFile* maxfs_get_file(int index) {
    if (index >= 0 && index < MAXFS_MAX_FILES && ram_disk[index].exists) {
        return &ram_disk[index];
    }
    return 0;
}

int maxfs_find_file(const char* name) {
    for (int i = 0; i < MAXFS_MAX_FILES; i++) {
        if (ram_disk[i].exists) {
            int match = 1;
            int n = 0;
            while (name[n] != '\0' || ram_disk[i].name[n] != '\0') {
                if (name[n] != ram_disk[i].name[n]) {
                    match = 0;
                    break;
                }
                n++;
            }
            if (match) return i;
        }
    }
    return -1;
}

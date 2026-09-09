#ifndef MAXB_H
#define MAXB_H

#define MAXB_MAGIC 0x4258414D  /* "MAXB" - maxOS Application Binary */
#define MAXB_VERSION 1

#define MAXB_APP_NONE      0
#define MAXB_APP_NOTEPAD   1
#define MAXB_APP_EXPLORER  2
#define MAXB_APP_CALC      3
#define MAXB_APP_SYSINFO   4
#define MAXB_APP_PONG      5
#define MAXB_APP_INSTALLER 6
#define MAXB_APP_MEM       7
#define MAXB_APP_STRESS    8
#define MAXB_APP_CUSTOM    9

// 64-byte Standalone Binary Executable Header
typedef struct __attribute__((packed)) {
    unsigned int magic;          // 0x4258414D ("MAXB")
    unsigned short version;      // 1
    unsigned short app_type;     // MAXB_APP_*
    char name[24];               // "Notepad", "Calculator", etc.
    char icon[8];                // "NP", "CALC", etc.
    unsigned short icon_color;   // RGB565 color
    unsigned short flags;        // 1 = has_tick
    unsigned int state_size;     // Size of instance state memory required (bytes)
    unsigned int binary_size;    // Total size of binary file (bytes)
    unsigned int entry_point;    // Entry/init offset
    unsigned int draw_offset;    // Draw handler offset
    unsigned int click_offset;   // Click handler offset
    unsigned int key_offset;     // Key handler offset
    unsigned int tick_offset;    // Optional tick handler offset
} maxb_header_t;

#endif // MAXB_H

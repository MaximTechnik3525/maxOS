#ifndef VERBOSE_BOOT_H
#define VERBOSE_BOOT_H

#define BOOT_STATUS_NONE 0
#define BOOT_STATUS_OK   1
#define BOOT_STATUS_SKIP 2
#define BOOT_STATUS_INFO 3
#define BOOT_STATUS_WARN 4
#define BOOT_STATUS_FAIL 5
#define BOOT_STATUS_DONE 6

// Initialize and manage verbose boot system
void verbose_boot_init(int enable, unsigned int mem_upper);
int  verbose_boot_is_active(void);
void verbose_boot_set_active(int active);
void verbose_boot_toggle(void);

// Logging and progress display
void verbose_boot_log(const char* tag, const char* msg);
void verbose_boot_step(const char* tag, const char* msg, int status_code, int percent);
void verbose_boot_progress(int percent, const char* label);
void verbose_boot_check_keys(void);
void verbose_boot_finish(void);

#endif // VERBOSE_BOOT_H

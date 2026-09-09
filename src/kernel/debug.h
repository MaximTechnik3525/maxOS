#ifndef DEBUG_H
#define DEBUG_H

// Core debug functions
void debug_init(void);
void debug_putc(char c);
void debug_puts(const char* s);
void debug_print_num(unsigned long long num, int base);
void debug_log(const char* tag, const char* msg);
void debug_log_app_event(const char* app_name, const char* event, int app_id);
void debug_log_ata_event(const char* op, unsigned int lba, int count, int status);

// Circular ring buffer of recent logs for in-OS viewing
#define DEBUG_HISTORY_COUNT 16
#define DEBUG_MSG_LEN 72

struct DebugLogEntry {
    unsigned long long timestamp_ms;
    char tag[8];
    char msg[DEBUG_MSG_LEN];
};

extern struct DebugLogEntry debug_history[DEBUG_HISTORY_COUNT];
extern int debug_history_head;
extern int debug_history_total;

const struct DebugLogEntry* debug_get_entry(int index);

#endif // DEBUG_H

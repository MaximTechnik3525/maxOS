#include "debug.h"
#include "kernel.h"
#include "idt.h"
#include "string.h"

#define COM1_PORT 0x3F8

static int serial_initialized = 0;

struct DebugLogEntry debug_history[DEBUG_HISTORY_COUNT];
int debug_history_head = 0;
int debug_history_total = 0;

void debug_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    serial_initialized = 1;

    debug_puts("\n========================================\n");
    debug_puts(" maxOS v4.0 EventUpdate x86_64 Debug Stream\n");
    debug_puts(" Diagnostic Logger active on COM1\n");
    debug_puts("========================================\n\n");
}

static int is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void debug_putc(char c) {
    if (!serial_initialized) return;
    int timeout = 10000;
    while (!is_transmit_empty() && --timeout);
    outb(COM1_PORT, c);
}

void debug_puts(const char* s) {
    while (*s) {
        if (*s == '\n') debug_putc('\r');
        debug_putc(*s);
        s++;
    }
}

void debug_print_num(unsigned long long num, int base) {
    char buf[32];
    int p = 0;
    if (num == 0) {
        debug_putc('0');
        return;
    }
    while (num > 0) {
        int rem = num % base;
        buf[p++] = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        num /= base;
    }
    while (p > 0) {
        debug_putc(buf[--p]);
    }
}

void debug_log(const char* tag, const char* msg) {
    unsigned long long ms = system_ticks;
    unsigned int sec = (unsigned int)(ms / 1000);
    unsigned int rem_ms = (unsigned int)(ms % 1000);

    // 1. Output to serial COM1
    debug_puts("[");
    debug_print_num(sec / 60, 10);
    debug_putc(':');
    if ((sec % 60) < 10) debug_putc('0');
    debug_print_num(sec % 60, 10);
    debug_putc('.');
    if (rem_ms < 100) debug_putc('0');
    if (rem_ms < 10) debug_putc('0');
    debug_print_num(rem_ms, 10);
    debug_puts("] [");
    debug_puts(tag);
    debug_puts("] ");
    debug_puts(msg);
    debug_puts("\n");

    // 2. Store in circular buffer for UI display
    struct DebugLogEntry* entry = &debug_history[debug_history_head];
    entry->timestamp_ms = ms;
    strncpy(entry->tag, tag, sizeof(entry->tag) - 1);
    entry->tag[sizeof(entry->tag) - 1] = '\0';
    strncpy(entry->msg, msg, sizeof(entry->msg) - 1);
    entry->msg[sizeof(entry->msg) - 1] = '\0';

    debug_history_head = (debug_history_head + 1) % DEBUG_HISTORY_COUNT;
    if (debug_history_total < DEBUG_HISTORY_COUNT) debug_history_total++;
}

void debug_log_app_event(const char* app_name, const char* event, int app_id) {
    char buf[DEBUG_MSG_LEN];
    int p = 0;
    const char* prefix = "App '";
    while (*prefix && p < DEBUG_MSG_LEN - 1) buf[p++] = *prefix++;
    while (*app_name && p < 25) buf[p++] = *app_name++;
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = '\'';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ' ';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = '(';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'I';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'D';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ' ';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = '0' + (app_id % 10);
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ')';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ':';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ' ';
    while (*event && p < DEBUG_MSG_LEN - 1) buf[p++] = *event++;
    buf[p] = '\0';
    debug_log("APP", buf);
}

void debug_log_ata_event(const char* op, unsigned int lba, int count, int status) {
    char buf[DEBUG_MSG_LEN];
    int p = 0;
    while (*op && p < 12) buf[p++] = *op++;
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = ' ';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'L';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'B';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'A';
    if (p < DEBUG_MSG_LEN - 1) buf[p++] = '=';

    char numbuf[16];
    int_str((int)lba, numbuf);
    int np = 0;
    while (numbuf[np] && p < DEBUG_MSG_LEN - 1) buf[p++] = numbuf[np++];

    if (count > 0) {
        if (p < DEBUG_MSG_LEN - 1) buf[p++] = ' ';
        if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'c';
        if (p < DEBUG_MSG_LEN - 1) buf[p++] = 'n';
        if (p < DEBUG_MSG_LEN - 1) buf[p++] = 't';
        if (p < DEBUG_MSG_LEN - 1) buf[p++] = '=';
        int_str(count, numbuf);
        np = 0;
        while (numbuf[np] && p < DEBUG_MSG_LEN - 1) buf[p++] = numbuf[np++];
    }

    if (p < DEBUG_MSG_LEN - 4) {
        buf[p++] = ' ';
        buf[p++] = '-';
        buf[p++] = '>';
        buf[p++] = ' ';
    }
    if (status == 0) {
        if (p < DEBUG_MSG_LEN - 3) {
            buf[p++] = 'O';
            buf[p++] = 'K';
        }
    } else {
        if (p < DEBUG_MSG_LEN - 5) {
            buf[p++] = 'E';
            buf[p++] = 'R';
            buf[p++] = 'R';
        }
    }
    buf[p] = '\0';
    debug_log("ATA", buf);
}

const struct DebugLogEntry* debug_get_entry(int index) {
    if (index < 0 || index >= debug_history_total) return 0;
    int idx = (debug_history_head - debug_history_total + index + DEBUG_HISTORY_COUNT) % DEBUG_HISTORY_COUNT;
    return &debug_history[idx];
}

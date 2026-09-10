#ifndef TASK_H
#define TASK_H

#include "string.h"

#define MAX_TASKS 16
#define TASK_STACK_SIZE 32768  // 32 KB per task
#define DEFAULT_TIME_SLICE 20  // 20 ms per quantum at 1000 Hz PIT

#define TIME_SLICE_LOW    10
#define TIME_SLICE_NORMAL 20
#define TIME_SLICE_HIGH   30

typedef enum {
    TASK_UNUSED = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_DEAD
} task_state_t;

typedef enum {
    TASK_PRIORITY_LOW    = 0,
    TASK_PRIORITY_NORMAL = 1,
    TASK_PRIORITY_HIGH   = 2
} task_priority_t;

// Complete CPU state saved on stack during interrupt / context switch
struct __attribute__((packed)) trap_frame {
    // Pushed by irq0_timer_entry (in order: r15 down to rax)
    unsigned long long r15;
    unsigned long long r14;
    unsigned long long r13;
    unsigned long long r12;
    unsigned long long r11;
    unsigned long long r10;
    unsigned long long r9;
    unsigned long long r8;
    unsigned long long rbp;
    unsigned long long rdi;
    unsigned long long rsi;
    unsigned long long rdx;
    unsigned long long rcx;
    unsigned long long rbx;
    unsigned long long rax;

    // Pushed automatically by CPU on interrupt / exception:
    unsigned long long rip;
    unsigned long long cs;
    unsigned long long rflags;
    unsigned long long rsp;
    unsigned long long ss;
};

typedef struct {
    int pid;
    char name[32];
    task_state_t state;
    task_priority_t priority;
    unsigned long long rsp;              // Saved RSP pointing to trap_frame
    unsigned long cr3;                   // PML4 Page Table base address
    unsigned long long kstack_top;       // Top of kernel stack for TSS.rsp0
    unsigned char kstack[TASK_STACK_SIZE] __attribute__((aligned(16)));
    unsigned char ustack[TASK_STACK_SIZE] __attribute__((aligned(16)));
    unsigned char fpu_state[512] __attribute__((aligned(16)));
    unsigned long long sleep_until;      // System ticks to wake up
    
    // Process-level event queue
    struct {
        int events[32 * 5]; // 32 events * 5 ints
        int head;
        int tail;
    } eq;

    int time_slice;                      // Remaining ticks in current quantum
    int is_user;                         // 1 = Ring 3, 0 = Ring 0
    int app_id;                          // Associated app_id (MAXP_APP_*) or 0
    unsigned long long total_ticks;      // Total CPU ticks consumed
} task_t;

// Task information for process listing / Task Manager
typedef struct {
    int pid;
    char name[32];
    int state;
    int is_user;
    int app_id;
    unsigned long long total_ticks;
    int priority;
} task_info_t;

// Core Task Lifecycle
void task_init(void);
int  task_create(const char* name, void (*entry)(void), int is_user, int app_id);
void task_exit(void);
int  task_kill(int pid);
void task_sleep(unsigned long long ms);
void task_yield(void);

// Task Attributes & Queries
void            task_set_priority(int pid, task_priority_t prio);
task_priority_t task_get_priority(int pid);
task_t*         task_get_current(void);
task_t*         task_get_by_pid(int pid);
int             task_get_count(void);
int             task_get_list(task_info_t* list, int max_count);
int             task_is_scheduler_active(void);
void            task_push_event(int pid, int type, int x, int y, int key, int scan);

// Called by irq0_timer_entry assembly
unsigned long long schedule_tick(unsigned long long current_rsp);

#endif // TASK_H

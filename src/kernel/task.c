#include "task.h"
#include "kernel.h"
#include "user.h"
#include "idt.h"
#include "debug.h"
#include "string.h"
#include "user/syscall.h"

static task_t tasks[MAX_TASKS];
static task_t* current_task = 0;
static int scheduler_active = 0;
static unsigned char default_fpu[512] __attribute__((aligned(16)));
static int fpu_initialized = 0;

extern unsigned long long interrupt_stack_top;

static void init_fpu_template(void) {
    if (!fpu_initialized) {
        __asm__ __volatile__("fninit");
        __asm__ __volatile__("fxsave64 %0" : "=m"(default_fpu));
        fpu_initialized = 1;
    }
}

static inline int sched_get_quantum(task_priority_t prio) {
    switch (prio) {
        case TASK_PRIORITY_HIGH:   return TIME_SLICE_HIGH;
        case TASK_PRIORITY_LOW:    return TIME_SLICE_LOW;
        case TASK_PRIORITY_NORMAL:
        default:                   return TIME_SLICE_NORMAL;
    }
}

void task_init(void) {
    init_fpu_template();

    memset(tasks, 0, sizeof(tasks));
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].pid = i;
        tasks[i].state = TASK_UNUSED;
        tasks[i].priority = TASK_PRIORITY_NORMAL;
        tasks[i].time_slice = DEFAULT_TIME_SLICE;
    }

    // Task 0: Main kernel & desktop compositor thread
    tasks[0].pid = 0;
    strncpy(tasks[0].name, "kernel_main", sizeof(tasks[0].name) - 1);
    tasks[0].state = TASK_RUNNING;
    tasks[0].priority = TASK_PRIORITY_HIGH; // High priority for smooth UI and desktop
    tasks[0].is_user = 0;
    tasks[0].time_slice = sched_get_quantum(TASK_PRIORITY_HIGH);
    tasks[0].kstack_top = (unsigned long long)&interrupt_stack_top;
    memcpy(tasks[0].fpu_state, default_fpu, sizeof(default_fpu));

    current_task = &tasks[0];
    scheduler_active = 1;

    debug_log("SCHED", "Preemptive Multitasking Scheduler Initialized (PID 0 active)");
}

int task_create(const char* name, void (*entry)(void), int is_user, int app_id) {
    if (get_cpl() == 3) {
        return u_spawn(name, entry, is_user, app_id);
    }

    // Disable interrupts to ensure atomic task creation
    unsigned long long flags;
    __asm__ __volatile__("pushfq; pop %0; cli" : "=r"(flags));

    int pid = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED || tasks[i].state == TASK_DEAD) {
            pid = i;
            break;
        }
    }
    if (pid == -1) {
        __asm__ __volatile__("push %0; popfq" : : "r"(flags));
        debug_log("SCHED", "Task table full! Cannot spawn new process.");
        return -1;
    }

    task_t* t = &tasks[pid];
    memset(t, 0, sizeof(task_t));

    t->pid = pid;
    t->state = TASK_UNUSED; // Kept UNUSED until stack and context are completely prepared
    t->is_user = is_user;
    t->app_id = app_id;
    t->priority = TASK_PRIORITY_NORMAL;
    t->time_slice = sched_get_quantum(TASK_PRIORITY_NORMAL);
    t->total_ticks = 0;
    t->sleep_until = 0;

    strncpy(t->name, name ? name : "task", sizeof(t->name) - 1);
    memcpy(t->fpu_state, default_fpu, sizeof(default_fpu));

    unsigned char* kstack_top = t->kstack + TASK_STACK_SIZE;
    t->kstack_top = (unsigned long long)kstack_top;

    struct trap_frame* tf = (struct trap_frame*)(kstack_top - sizeof(struct trap_frame));
    memset(tf, 0, sizeof(struct trap_frame));

    if (is_user && entry != 0) {
        // Real Ring 3 user process
        unsigned char* ustack_top = t->ustack + TASK_STACK_SIZE - 16;
        tf->ss = 0x23; // User DS/SS (0x20 | 3)
        tf->rsp = (unsigned long long)ustack_top;
        tf->rflags = 0x202; // IF=1
        tf->cs = 0x2B; // User CS (0x28 | 3)
        tf->rip = (unsigned long long)entry;
        t->state = TASK_READY;
    } else if (entry != 0) {
        // Kernel task or background worker
        unsigned char* k_rsp = (unsigned char*)(kstack_top - sizeof(struct trap_frame) - 16);
        *((unsigned long long*)k_rsp) = (unsigned long long)task_exit;

        tf->ss = 0x10; // Kernel DS/SS
        tf->rsp = (unsigned long long)k_rsp;
        tf->rflags = 0x202; // IF=1
        tf->cs = 0x08; // Kernel CS
        tf->rip = (unsigned long long)entry;
        t->state = TASK_READY;
    } else {
        // UI Application process (event-driven, managed by desktop compositor)
        // Kept in SLEEPING state so it does not waste CPU cycles or trigger hlt
        t->state = TASK_SLEEPING;
        t->sleep_until = 0xFFFFFFFFFFFFFFFFULL;
    }

    t->rsp = (unsigned long long)tf;

    // Restore interrupt state
    __asm__ __volatile__("push %0; popfq" : : "r"(flags));

    debug_log_app_event("SCHED", "Created Process", pid);
    return pid;
}

void task_exit(void) {
    if (current_task && current_task->pid != 0) {
        current_task->state = TASK_DEAD;
        current_task->rsp = 0;
        debug_log_app_event("SCHED", "Process Terminated", current_task->pid);
    }
    task_yield();
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

int task_kill(int pid) {
    if (get_cpl() == 3) {
        return u_kill(pid);
    }
    if (pid <= 0 || pid >= MAX_TASKS) return 0;
    if (tasks[pid].state == TASK_UNUSED || tasks[pid].state == TASK_DEAD) return 0;

    tasks[pid].state = TASK_DEAD;
    tasks[pid].rsp = 0;
    debug_log_app_event("SCHED", "Killed Process", pid);

    if (current_task == &tasks[pid]) {
        task_yield();
    }
    return 1;
}

void task_sleep(unsigned long long ms) {
    if (get_cpl() == 3) {
        u_sleep((unsigned int)ms);
        return;
    }
    if (!scheduler_active || !current_task) {
        sleep((unsigned int)ms);
        return;
    }

    current_task->sleep_until = get_uptime_ms() + ms;
    current_task->state = TASK_SLEEPING;
    current_task->time_slice = 0;

    while (current_task->state == TASK_SLEEPING) {
        task_yield();
    }
}

void task_yield(void) {
    if (get_cpl() == 3) {
        u_yield();
        return;
    }
    if (current_task) {
        current_task->time_slice = 0;
    }
    __asm__ __volatile__("pause");
}

void task_set_priority(int pid, task_priority_t prio) {
    if (pid >= 0 && pid < MAX_TASKS) {
        tasks[pid].priority = prio;
    }
}

task_priority_t task_get_priority(int pid) {
    if (pid >= 0 && pid < MAX_TASKS) {
        return tasks[pid].priority;
    }
    return TASK_PRIORITY_NORMAL;
}

task_t* task_get_current(void) {
    return current_task;
}

task_t* task_get_by_pid(int pid) {
    if (pid < 0 || pid >= MAX_TASKS) return 0;
    return &tasks[pid];
}

int task_get_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state != TASK_UNUSED && tasks[i].state != TASK_DEAD) {
            count++;
        }
    }
    return count;
}

int task_get_list(task_info_t* list, int max_count) {
    if (get_cpl() == 3) {
        return u_tasklist(list, max_count);
    }
    int out_count = 0;
    for (int i = 0; i < MAX_TASKS && out_count < max_count; i++) {
        if (tasks[i].state != TASK_UNUSED) {
            list[out_count].pid = tasks[i].pid;
            strncpy(list[out_count].name, tasks[i].name, sizeof(list[out_count].name) - 1);
            list[out_count].name[sizeof(list[out_count].name) - 1] = '\0';
            list[out_count].state = (int)tasks[i].state;
            list[out_count].is_user = tasks[i].is_user;
            list[out_count].app_id = tasks[i].app_id;
            list[out_count].total_ticks = tasks[i].total_ticks;
            list[out_count].priority = (int)tasks[i].priority;
            out_count++;
        }
    }
    return out_count;
}

int task_is_scheduler_active(void) {
    return scheduler_active;
}

/* -------------------------------------------------------------------------
 * Internal Scheduler Routines
 * ------------------------------------------------------------------------- */
static void sched_wake_sleeping(unsigned long long now_ticks) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_SLEEPING) {
            if (now_ticks >= tasks[i].sleep_until) {
                tasks[i].state = TASK_READY;
            }
        }
    }
}

static int sched_pick_next(task_t* cur) {
    int start = cur ? cur->pid : 0;

    // Scan priority levels from HIGH down to LOW
    for (int p = (int)TASK_PRIORITY_HIGH; p >= (int)TASK_PRIORITY_LOW; p--) {
        for (int i = 1; i <= MAX_TASKS; i++) {
            int idx = (start + i) % MAX_TASKS;
            if (tasks[idx].state == TASK_READY && tasks[idx].priority == (task_priority_t)p) {
                return idx;
            }
        }
    }

    return -1;
}

static void sched_switch_fpu(task_t* cur, task_t* next) {
    if (cur != next) {
        if (cur && cur->state != TASK_DEAD && cur->state != TASK_UNUSED) {
            __asm__ __volatile__("fxsave64 %0" : "=m"(cur->fpu_state));
        }
        __asm__ __volatile__("fxrstor64 %0" : : "m"(next->fpu_state));
    }
}

/* -------------------------------------------------------------------------
 * Preemptive Scheduler Quantum Tick (called from irq0_timer_entry)
 * ------------------------------------------------------------------------- */
unsigned long long schedule_tick(unsigned long long current_rsp) {
    system_ticks++;

    if (!scheduler_active) {
        return current_rsp;
    }

    // 1. Wake up sleeping tasks whose delay has elapsed
    sched_wake_sleeping(system_ticks);

    task_t* cur = current_task;
    if (cur) {
        cur->total_ticks++;
        if (cur->state == TASK_RUNNING) {
            cur->time_slice--;
            if (cur->time_slice > 0) {
                // Quantum is still active for current task
                return current_rsp;
            }
            cur->state = TASK_READY;
        }
        cur->rsp = current_rsp;
    }

    // 2. Select next READY task using Priority-Aware Round-Robin
    int next_idx = sched_pick_next(cur);

    if (next_idx == -1) {
        // No other task ready: if current task is still READY, resume it
        if (cur && cur->state == TASK_READY) {
            cur->state = TASK_RUNNING;
            cur->time_slice = sched_get_quantum(cur->priority);
            return cur->rsp;
        }

        // Fallback to task 0 (kernel compositor / idle)
        if (tasks[0].state != TASK_DEAD && tasks[0].state != TASK_UNUSED) {
            sched_switch_fpu(cur, &tasks[0]);
            tasks[0].state = TASK_RUNNING;
            tasks[0].time_slice = sched_get_quantum(tasks[0].priority);
            current_task = &tasks[0];
            tss_set_rsp0(tasks[0].kstack_top);
            return (tasks[0].rsp != 0) ? tasks[0].rsp : current_rsp;
        }
        return current_rsp;
    }

    // 3. Switch context to selected task
    task_t* next = &tasks[next_idx];
    sched_switch_fpu(cur, next);

    next->state = TASK_RUNNING;
    next->time_slice = sched_get_quantum(next->priority);
    current_task = next;

    // Load next task's kernel stack top into TSS.rsp0 for privilege transitions
    tss_set_rsp0(next->kstack_top);

    return next->rsp;
}

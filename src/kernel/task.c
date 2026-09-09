#include "task.h"
#include "kernel.h"
#include "user.h"
#include "idt.h"
#include "debug.h"

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

void task_init(void) {
    init_fpu_template();

    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].pid = i;
        tasks[i].state = TASK_UNUSED;
        tasks[i].rsp = 0;
        tasks[i].kstack_top = 0;
        tasks[i].sleep_until = 0;
        tasks[i].time_slice = DEFAULT_TIME_SLICE;
        tasks[i].is_user = 0;
        tasks[i].app_id = 0;
        tasks[i].total_ticks = 0;
        tasks[i].name[0] = '\0';
    }

    // Task 0: Main kernel & desktop compositor thread
    tasks[0].pid = 0;
    const char* k_name = "kernel_main";
    for (int i = 0; k_name[i] != '\0' && i < 31; i++) {
        tasks[0].name[i] = k_name[i];
        tasks[0].name[i + 1] = '\0';
    }
    tasks[0].state = TASK_RUNNING;
    tasks[0].is_user = 0;
    tasks[0].time_slice = DEFAULT_TIME_SLICE;
    tasks[0].kstack_top = (unsigned long long)&interrupt_stack_top;
    
    // Copy clean FPU state
    for (int j = 0; j < 512; j++) {
        tasks[0].fpu_state[j] = default_fpu[j];
    }

    current_task = &tasks[0];
    scheduler_active = 1;

    debug_log("SCHED", "Preemptive Multitasking Scheduler Initialized (PID 0 active)");
}

int task_create(const char* name, void (*entry)(void), int is_user, int app_id) {
    int pid = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED || tasks[i].state == TASK_DEAD) {
            pid = i;
            break;
        }
    }
    if (pid == -1) {
        debug_log("SCHED", "Task table full! Cannot spawn new process.");
        return -1;
    }

    task_t* t = &tasks[pid];
    t->pid = pid;
    t->state = TASK_READY;
    t->is_user = is_user;
    t->app_id = app_id;
    t->time_slice = DEFAULT_TIME_SLICE;
    t->total_ticks = 0;
    t->sleep_until = 0;

    int n = 0;
    while (name && name[n] != '\0' && n < 31) {
        t->name[n] = name[n];
        n++;
    }
    t->name[n] = '\0';

    // Copy clean FPU state
    for (int j = 0; j < 512; j++) {
        t->fpu_state[j] = default_fpu[j];
    }

    unsigned char* kstack_top = t->kstack + TASK_STACK_SIZE;
    t->kstack_top = (unsigned long long)kstack_top;

    struct trap_frame* tf = (struct trap_frame*)(kstack_top - sizeof(struct trap_frame));

    tf->rax = 0;
    tf->rbx = 0;
    tf->rcx = 0;
    tf->rdx = 0;
    tf->rsi = 0;
    tf->rdi = 0;
    tf->rbp = 0;
    tf->r8  = 0;
    tf->r9  = 0;
    tf->r10 = 0;
    tf->r11 = 0;
    tf->r12 = 0;
    tf->r13 = 0;
    tf->r14 = 0;
    tf->r15 = 0;

    if (is_user) {
        // Ring 3 process
        unsigned char* ustack_top = t->ustack + TASK_STACK_SIZE - 16;
        tf->ss = 0x23; // User DS/SS (0x20 | 3)
        tf->rsp = (unsigned long long)ustack_top;
        tf->rflags = 0x202; // IF=1
        tf->cs = 0x2B; // User CS (0x28 | 3)
        tf->rip = (unsigned long long)entry;
    } else {
        // Ring 0 kernel task
        unsigned char* k_rsp = (unsigned char*)(kstack_top - sizeof(struct trap_frame) - 16);
        // Put task_exit address as return pointer if entry returns
        *((unsigned long long*)k_rsp) = (unsigned long long)task_exit;

        tf->ss = 0x10; // Kernel DS/SS
        tf->rsp = (unsigned long long)k_rsp;
        tf->rflags = 0x202; // IF=1
        tf->cs = 0x08; // Kernel CS
        tf->rip = (unsigned long long)entry;
    }

    t->rsp = (unsigned long long)tf;

    debug_log_app_event("SCHED", "Created Process", pid);
    return pid;
}

void task_exit(void) {
    if (current_task && current_task->pid != 0) {
        current_task->state = TASK_DEAD;
        debug_log_app_event("SCHED", "Process Terminated", current_task->pid);
    }
    task_yield();
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

int task_kill(int pid) {
    if (pid <= 0 || pid >= MAX_TASKS) return 0;
    if (tasks[pid].state == TASK_UNUSED || tasks[pid].state == TASK_DEAD) return 0;

    tasks[pid].state = TASK_DEAD;
    debug_log_app_event("SCHED", "Killed Process", pid);

    if (current_task == &tasks[pid]) {
        task_yield();
    }
    return 1;
}

void task_sleep(unsigned long long ms) {
    if (!scheduler_active || !current_task) {
        sleep((unsigned int)ms);
        return;
    }

    current_task->sleep_until = get_uptime_ms() + ms;
    current_task->state = TASK_SLEEPING;
    current_task->time_slice = 0;

    while (current_task->state == TASK_SLEEPING) {
        __asm__ __volatile__("sti; hlt");
    }
}

void task_yield(void) {
    if (current_task) {
        current_task->time_slice = 0;
    }
    __asm__ __volatile__("sti; hlt");
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
    int out_count = 0;
    for (int i = 0; i < MAX_TASKS && out_count < max_count; i++) {
        if (tasks[i].state != TASK_UNUSED) {
            list[out_count].pid = tasks[i].pid;
            for (int k = 0; k < 32; k++) {
                list[out_count].name[k] = tasks[i].name[k];
            }
            list[out_count].state = (int)tasks[i].state;
            list[out_count].is_user = tasks[i].is_user;
            list[out_count].app_id = tasks[i].app_id;
            list[out_count].total_ticks = tasks[i].total_ticks;
            out_count++;
        }
    }
    return out_count;
}

int task_is_scheduler_active(void) {
    return scheduler_active;
}

unsigned long long schedule_tick(unsigned long long current_rsp) {
    system_ticks++;

    if (!scheduler_active) {
        return current_rsp;
    }

    // 1. Wake up sleeping tasks
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_SLEEPING) {
            if (system_ticks >= tasks[i].sleep_until) {
                tasks[i].state = TASK_READY;
            }
        }
    }

    task_t* cur = current_task;
    if (cur) {
        cur->total_ticks++;
        if (cur->state == TASK_RUNNING) {
            cur->time_slice--;
            if (cur->time_slice > 0) {
                // Current task's quantum is still active
                return current_rsp;
            }
            cur->state = TASK_READY;
        }
        cur->rsp = current_rsp;
    }

    // 2. Select next READY task using Round-Robin
    int start = cur ? cur->pid : 0;
    int next_idx = -1;

    for (int i = 1; i <= MAX_TASKS; i++) {
        int idx = (start + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_READY) {
            next_idx = idx;
            break;
        }
    }

    if (next_idx == -1) {
        // No other task is ready
        if (cur && cur->state == TASK_READY) {
            cur->state = TASK_RUNNING;
            cur->time_slice = DEFAULT_TIME_SLICE;
            return cur->rsp;
        }
        // Fallback to task 0 (desktop/kernel)
        if (tasks[0].state != TASK_DEAD && tasks[0].state != TASK_UNUSED) {
            if (cur != &tasks[0]) {
                if (cur) {
                    __asm__ __volatile__("fxsave64 %0" : "=m"(cur->fpu_state));
                }
                __asm__ __volatile__("fxrstor64 %0" : : "m"(tasks[0].fpu_state));
            }
            tasks[0].state = TASK_RUNNING;
            tasks[0].time_slice = DEFAULT_TIME_SLICE;
            current_task = &tasks[0];
            tss_set_rsp0(tasks[0].kstack_top);
            return tasks[0].rsp;
        }
        return current_rsp;
    }

    // 3. Switch to selected task
    task_t* next = &tasks[next_idx];

    if (cur != next) {
        if (cur) {
            __asm__ __volatile__("fxsave64 %0" : "=m"(cur->fpu_state));
        }
        __asm__ __volatile__("fxrstor64 %0" : : "m"(next->fpu_state));
    }

    next->state = TASK_RUNNING;
    next->time_slice = DEFAULT_TIME_SLICE;
    current_task = next;

    // Load next task's kernel stack into TSS.rsp0 for Ring 3 privilege transitions
    tss_set_rsp0(next->kstack_top);

    return next->rsp;
}

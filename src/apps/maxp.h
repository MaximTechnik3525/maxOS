#ifndef MAXP_H
#define MAXP_H

#include "maxb.h"

#define MAXP_APP_NONE      0
#define MAXP_APP_NOTEPAD   1
#define MAXP_APP_EXPLORER  2
#define MAXP_APP_CALC      3
#define MAXP_APP_SYSINFO   4
#define MAXP_APP_PONG      5
#define MAXP_APP_INSTALLER 6
#define MAXP_APP_MEM       7
#define MAXP_APP_STRESS    8
#define MAXP_APP_CUSTOM    9

#define MAXP_APP_COUNT     8
#define MAX_APP_INSTANCES  16

// Application instance structure
typedef struct {
    int instance_id;        // 1..MAX_APP_INSTANCES (0 = inactive)
    int pid;                // Scheduler PID
    int app_type;           // MAXP_APP_*
    char title[32];         // "Notepad #1", "Calculator #2", etc.
    char icon[8];           // "NP", "CALC", etc.
    unsigned short icon_color;
    
    // Window geometry (per instance)
    int win_x;
    int win_y;
    int win_w;
    int win_h;
    int is_minimized;
    
    // Instance-private state
    void* state;
    unsigned int state_size;
    
    // Function hooks
    void (*draw)(void* state, int x, int y, int w, int h);
    int  (*handle_click)(void* state, int x, int y, int w, int h, int mx, int my);
    int  (*handle_key)(void* state, char ch, unsigned char scan);
    void (*tick)(void* state, int x, int y, int w, int h);
    void (*close)(void* state);
} app_instance_t;

struct MaxPAppInfo {
    int id;
    const char* name;
    const char* filename;
    const char* desc;
    const char* icon_code;
    unsigned short icon_color;
};

void maxp_init(void);
int maxp_is_maxp_file(const char* filename);
int maxp_launch_file(const char* filename);

// Multi-instance lifecycle
int maxp_spawn_instance(int app_type, const char* custom_title, const char* file_arg);
int maxp_launch_app(int app_type); // Spawns new instance
int maxp_close_instance(int instance_id);
void maxp_close_app(int app_id);
void maxp_close_all_windows(void);

// Instance queries
int maxp_get_instance_count(void);
app_instance_t* maxp_get_instance(int instance_id);
app_instance_t* maxp_get_instance_by_index(int index);
app_instance_t* maxp_get_active_instance(void);
int maxp_get_active_instance_id(void);
void maxp_set_active_instance(int instance_id);
void maxp_cycle_active_instance(void);

// Drawing & input routing
void maxp_draw_active_instance(void);
void maxp_draw_all_instances(void);
int maxp_handle_click_active(int mx, int my);
int maxp_handle_key_active(char ch, unsigned char scan);
void maxp_tick_all_instances(void);
int  maxp_has_ticking_instances(void);

// Legacy compatibility
int maxp_get_active_app(void);
void maxp_set_active_app(int app_id);
int maxp_is_app_running(int app_id);
int maxp_get_running_apps(int* out_apps, int max_count);
int maxp_get_running_count(void);
const struct MaxPAppInfo* maxp_get_app_info(int app_id);
const struct MaxPAppInfo* maxp_get_app_by_index(int index);

// Standalone binary launcher
int maxp_launch_binary(const char* filename, const void* data, unsigned int size);

#endif // MAXP_H

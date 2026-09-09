#ifndef MAXP_H
#define MAXP_H

#define MAXP_APP_NONE      0
#define MAXP_APP_NOTEPAD   1
#define MAXP_APP_EXPLORER  2
#define MAXP_APP_CALC      3
#define MAXP_APP_SYSINFO   4
#define MAXP_APP_PONG      5
#define MAXP_APP_INSTALLER 6

#define MAXP_APP_COUNT     6

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
int maxp_launch_app(int app_id);
int maxp_get_active_app(void);
void maxp_set_active_app(int app_id);
void maxp_close_all_windows(void);
const struct MaxPAppInfo* maxp_get_app_info(int app_id);
const struct MaxPAppInfo* maxp_get_app_by_index(int index);

#endif // MAXP_H

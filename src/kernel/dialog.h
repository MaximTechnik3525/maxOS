#ifndef DIALOG_H
#define DIALOG_H

#define DIALOG_MAX_FIELDS 4
#define DIALOG_FIELD_LABEL_LEN 32
#define DIALOG_FIELD_VALUE_LEN 64

#define DIALOG_RES_NONE    0
#define DIALOG_RES_OK      1
#define DIALOG_RES_CANCEL -1

typedef struct {
    char label[DIALOG_FIELD_LABEL_LEN];
    char value[DIALOG_FIELD_VALUE_LEN];
    int len;
    int cursor;
    int max_len;
} dialog_field_t;

typedef struct {
    int active;
    int result;              // DIALOG_RES_NONE, DIALOG_RES_OK, DIALOG_RES_CANCEL
    int dialog_type;         // User tag / purpose ID
    int x, y, w, h;          // Window bounds
    char title[48];          // Window title
    char message[64];        // Header / description message
    
    dialog_field_t fields[DIALOG_MAX_FIELDS];
    int field_count;
    int focused_field;
    
    char ok_label[16];       // default "OK"
    char cancel_label[16];   // default "Cancel"
    
    int ok_btn_x, ok_btn_y, ok_btn_w, ok_btn_h;
    int cancel_btn_x, cancel_btn_y, cancel_btn_w, cancel_btn_h;
} dialog_t;

void dialog_init(dialog_t* dlg, int type, const char* title, const char* message);
void dialog_set_geometry(dialog_t* dlg, int x, int y, int w, int h);
void dialog_center(dialog_t* dlg, int parent_x, int parent_y, int parent_w, int parent_h);
int  dialog_add_field(dialog_t* dlg, const char* label, const char* default_val, int max_len);
void dialog_set_buttons(dialog_t* dlg, const char* ok_label, const char* cancel_label);
void dialog_show(dialog_t* dlg);
void dialog_close(dialog_t* dlg);

void dialog_draw(dialog_t* dlg);
int  dialog_handle_click(dialog_t* dlg, int mouse_x, int mouse_y);
int  dialog_handle_key(dialog_t* dlg, char ascii_char, unsigned char scan_code);

const char* dialog_get_field(dialog_t* dlg, int field_idx);
void dialog_set_field(dialog_t* dlg, int field_idx, const char* val);

#endif // DIALOG_H

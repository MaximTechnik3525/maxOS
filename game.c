#include "maxos_api.h"
void draw_player(maxos_api_t* api, int old_x, int old_y, int px, int py);
void _start(maxos_api_t* api) {
    int player_x = 0;
    int player_y = 0;
    int player_speed = 12;
    for (int y = 0; y < api->screen_h; y++) {
        for (int x = 0; x < api->screen_w; x++) {
            api->back_buffer[y * api->screen_w + x] = 0x10A2;
        }
    }
    while (1) {
        int old_x = player_x;
        int old_y = player_y;
        if (api->inb(0x64) & 0x01) {
            unsigned char sc = api->inb(0x60);
            if (sc == 0x01) {
                break;
            }
            if (sc == 0x11 && (player_y - player_speed) >= 0) { //W
                player_y -= player_speed;
            }
            if (sc == 0x1E && (player_x - player_speed) >= 0) { //A
                player_x -= player_speed;
            }
            if (sc == 0x1F && (player_y + player_speed) <= 708) { //S
                player_y += player_speed;
            }
            if (sc == 0x20 && (player_x + player_speed) <= 1268) { //D
                player_x += player_speed;
            }
            if (old_x != player_x || old_y != player_y) {
                draw_player(api, old_x, old_y, player_x, player_y);
            }
        }
        api->update_screen();
        api->sleep(16);
    }
}
void draw_player(maxos_api_t* api, int old_x, int old_y, int px, int py) {
    for (int y = old_y; y < old_y + 12; y++) {
        for (int x = old_x; x < old_x + 12; x++) {
            api->back_buffer[y * api->screen_w + x] = 0x10A2;
        }
    }
    for (int y = py; y < py + 12; y++) {
        for (int x = px; x < px + 12; x++) {
            api->back_buffer[y * api->screen_w + x] = 0x9000;
        }
    }
}
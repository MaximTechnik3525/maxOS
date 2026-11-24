#include "maxos_api.h"

void _start(maxos_api_t* api) {
    int px = 600;
    int py = 320;
    int size = 40;

    api->play_sound(523); api->sleep(70);
    api->play_sound(659); api->sleep(70);
    api->play_sound(784); api->sleep(100);
    api->no_sound();

    while (1) {
        for (int y = 0; y < api->screen_h; y++) {
            for (int x = 0; x < api->screen_w; x++) {
                api->back_buffer[y * api->screen_w + x] = 0x0841;
            }
        }

        api->print_string("EXTERNAL ELF APP RUNNING IN MAXOS!", 20, 20, 0x07FF);
        api->print_string("Use [W A S D] to move, [ESC] to return to maxOS Desktop", 20, 40, 0xFFFF);

        for (int y = py; y < py + size; y++) {
            for (int x = px; x < px + size; x++) {
                api->back_buffer[y * api->screen_w + x] = 0xF81F;
            }
        }

        api->update_screen();

        unsigned char status = api->inb(0x64);
        if (status & 0x01) {
            if (status & 0x20) {
                api->inb(0x60);
                continue;
            }
            unsigned char sc = api->inb(0x60);
            if (sc == 0x01) { // ESC
                break;
            }
            if (sc == 0x11 && py > 60) { py -= 15; }  // W
            if (sc == 0x1F && py < 660) { py += 15; } // S
            if (sc == 0x1E && px > 20) { px -= 15; }  // A
            if (sc == 0x20 && px < 1220) { px += 15; } // D
        }
        api->sleep(15);
    }
}
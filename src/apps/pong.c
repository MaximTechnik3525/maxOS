#include "pong.h"
#include "maxp.h"
#include "kernel.h"

int pong_open = 0;
static pong_state_t primary_pong_state;

void pong_instance_init(pong_state_t* s, int px, int py, int pw, int ph) {
    s->pad_w = 90;
    s->pad_h = 12;
    s->pad_x = px + (pw / 2) - (s->pad_w / 2);
    s->pad_y = py + ph - 25;

    s->ball_x = px + (pw / 2);
    s->ball_y = py + 70;
    s->ball_dx = 4;
    s->ball_dy = 4;
    s->ball_size = 8;
    s->pong_score = 0;
    s->pong_high_score = 0;
    s->pong_game_over = 0;
}

void pong_instance_draw(pong_state_t* s, int px, int py, int pw, int ph) {
    prev_cursor();

    // Window frame
    draw_rect(px, py, pw, ph, 0x0000);
    draw_rect(px + 1, py + 1, pw - 2, ph - 2, 0xCE79);

    // Titlebar
    draw_rect(px + 2, py + 2, pw - 4, 22, 0x11EB);
    print_string("Pong Arcade - [pong.bin]", px + 8, py + 6, 0xFFFF);

    // [_] Minimize & [X] Close button
    draw_ui_btn(px + pw - 44, py + 4, 18, 16, "_", 0xCE79, 0x0000);
    draw_ui_btn(px + pw - 22, py + 4, 18, 16, "X", 0xF800, 0xFFFF);

    // Score & Info Bar
    draw_rect(px + 4, py + 26, pw - 8, 22, 0x0000);
    char s_str[16], hs_str[16];
    int_str(s->pong_score, s_str);
    int_str(s->pong_high_score, hs_str);

    print_string("Score: ", px + 12, py + 32, 0x07E0);
    print_string(s_str, px + 68, py + 32, 0xFFFF);

    print_string("High: ", px + 140, py + 32, 0xFCEF);
    print_string(hs_str, px + 190, py + 32, 0xFFFF);

    print_string("Controls: [A] Left  [D] Right  [R] Restart  [C] Exit", px + 270, py + 32, 0x24EE);

    // Playing Court
    int court_x = px + 6;
    int court_y = py + 52;
    int court_w = pw - 12;
    int court_h = ph - 58;

    draw_rect(court_x, court_y, court_w, court_h, 0x01A4);

    // Border line inside court
    draw_rect(court_x + 2, court_y + 2, court_w - 4, 2, 0xFFFF);
    draw_rect(court_x + 2, court_y + 2, 2, court_h - 4, 0xFFFF);
    draw_rect(court_x + court_w - 4, court_y + 2, 2, court_h - 4, 0xFFFF);

    if (s->pong_game_over) {
        print_string("GAME OVER! Press [R] to Play Again", px + (pw / 2) - 150, py + (ph / 2), 0xF800);
    } else {
        // Draw paddle
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x0000);
        draw_rect(s->pad_x + 1, s->pad_y + 1, s->pad_w - 2, 2, 0xFFFF);
        draw_rect(s->pad_x + 1, s->pad_y + 3, s->pad_w - 2, s->pad_h - 4, 0x24EE);

        // Draw ball
        draw_rect(s->ball_x, s->ball_y, s->ball_size, s->ball_size, 0xFFE0);
    }

    draw_cursor(pos_x, pos_y);
}

void pong_instance_tick(pong_state_t* s, int px, int py, int pw, int ph) {
    if (s->pong_game_over) return;

    int court_x = px + 6;
    int court_y = py + 52;
    int court_w = pw - 12;
    int court_h = ph - 58;

    // Erase old ball
    draw_rect(s->ball_x, s->ball_y, s->ball_size, s->ball_size, 0x01A4);

    // Update ball position
    s->ball_x += s->ball_dx;
    s->ball_y += s->ball_dy;

    // Bounce left wall
    if (s->ball_x <= court_x + 4) {
        s->ball_x = court_x + 5;
        s->ball_dx = -s->ball_dx;
        play_sound(600); sleep(5); no_sound();
    }
    // Bounce right wall
    if (s->ball_x + s->ball_size >= court_x + court_w - 4) {
        s->ball_x = court_x + court_w - 4 - s->ball_size;
        s->ball_dx = -s->ball_dx;
        play_sound(600); sleep(5); no_sound();
    }
    // Bounce top wall
    if (s->ball_y <= court_y + 4) {
        s->ball_y = court_y + 5;
        s->ball_dy = -s->ball_dy;
        play_sound(750); sleep(5); no_sound();
    }

    // Paddle collision
    if (s->ball_y + s->ball_size >= s->pad_y && s->ball_y <= s->pad_y + s->pad_h) {
        if (s->ball_x + s->ball_size >= s->pad_x && s->ball_x <= s->pad_x + s->pad_w) {
            s->ball_y = s->pad_y - s->ball_size;
            s->ball_dy = -s->ball_dy;
            s->pong_score += 10;
            if (s->pong_score > s->pong_high_score) s->pong_high_score = s->pong_score;
            play_sound(950); sleep(8); no_sound();

            // Refresh score header
            draw_rect(px + 4, py + 26, pw - 8, 22, 0x0000);
            char s_str[16], hs_str[16];
            int_str(s->pong_score, s_str);
            int_str(s->pong_high_score, hs_str);
            print_string("Score: ", px + 12, py + 32, 0x07E0);
            print_string(s_str, px + 68, py + 32, 0xFFFF);
            print_string("High: ", px + 140, py + 32, 0xFCEF);
            print_string(hs_str, px + 190, py + 32, 0xFFFF);
            print_string("Controls: [A] Left  [D] Right  [R] Restart  [C] Exit", px + 270, py + 32, 0x24EE);
        }
    }

    // Missed paddle: Game Over
    if (s->ball_y >= court_y + court_h) {
        s->pong_game_over = 1;
        play_sound(200); sleep(100); play_sound(150); sleep(120); no_sound();
        pong_instance_draw(s, px, py, pw, ph);
        return;
    }

    // Redraw ball
    draw_rect(s->ball_x, s->ball_y, s->ball_size, s->ball_size, 0xFFE0);
}

int pong_instance_click(pong_state_t* s, int px, int py, int pw, int ph, int mouse_x, int mouse_y) {
    // [_] Minimize button (Titlebar)
    if (mouse_x >= px + pw - 48 && mouse_x <= px + pw - 26 && mouse_y >= py && mouse_y <= py + 24) {
        ui_btn_click_effect(px + pw - 44, py + 4, 18, 16, "_", 0xCE79, 0x0000);
        return -2; // Request minimize
    }

    // [X] Close button (Titlebar)
    if (mouse_x >= px + pw - 26 && mouse_x <= px + pw && mouse_y >= py && mouse_y <= py + 24) {
        ui_btn_click_effect(px + pw - 22, py + 4, 18, 16, "X", 0xF800, 0xFFFF);
        return -1; // Request close
    }

    // Move paddle toward mouse if clicked inside court
    int court_x = px + 6;
    int court_w = pw - 12;
    if (mouse_x >= court_x && mouse_x <= court_x + court_w) {
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x01A4);
        s->pad_x = mouse_x - (s->pad_w / 2);
        if (s->pad_x < court_x + 4) s->pad_x = court_x + 4;
        if (s->pad_x + s->pad_w > court_x + court_w - 4) s->pad_x = court_x + court_w - 4 - s->pad_w;
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x0000);
        draw_rect(s->pad_x + 1, s->pad_y + 1, s->pad_w - 2, 2, 0xFFFF);
        draw_rect(s->pad_x + 1, s->pad_y + 3, s->pad_w - 2, s->pad_h - 4, 0x24EE);
    }

    // If game over, click restarts
    if (s->pong_game_over) {
        s->pong_score = 0;
        pong_instance_init(s, px, py, pw, ph);
        pong_instance_draw(s, px, py, pw, ph);
        return 1;
    }

    if (mouse_x >= px && mouse_x <= px + pw && mouse_y >= py && mouse_y <= py + ph) {
        return 1;
    }

    return 0;
}

int pong_instance_key(pong_state_t* s, char ascii_char, unsigned char scan_code) {
    app_instance_t* inst = maxp_get_active_instance();
    int px = inst ? inst->win_x : (win_x + 30);
    int py = inst ? inst->win_y : (win_y + 35);
    int pw = inst ? inst->win_w : (win_w - 60);
    int ph = inst ? inst->win_h : (win_h - 60);
    int court_x = px + 6;
    int court_w = pw - 12;

    // Left
    if (ascii_char == 'a' || ascii_char == 'A' || ascii_char == 'L' || scan_code == 0x4B) {
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x01A4);
        s->pad_x -= 30;
        if (s->pad_x < court_x + 4) s->pad_x = court_x + 4;
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x0000);
        draw_rect(s->pad_x + 1, s->pad_y + 1, s->pad_w - 2, 2, 0xFFFF);
        draw_rect(s->pad_x + 1, s->pad_y + 3, s->pad_w - 2, s->pad_h - 4, 0x24EE);
        return 1;
    }

    // Right
    if (ascii_char == 'd' || ascii_char == 'D' || ascii_char == 'R' || scan_code == 0x4D) {
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x01A4);
        s->pad_x += 30;
        if (s->pad_x + s->pad_w > court_x + court_w - 4) s->pad_x = court_x + court_w - 4 - s->pad_w;
        draw_rect(s->pad_x, s->pad_y, s->pad_w, s->pad_h, 0x0000);
        draw_rect(s->pad_x + 1, s->pad_y + 1, s->pad_w - 2, 2, 0xFFFF);
        draw_rect(s->pad_x + 1, s->pad_y + 3, s->pad_w - 2, s->pad_h - 4, 0x24EE);
        return 1;
    }

    // Restart
    if (ascii_char == 'r' || ascii_char == 'R') {
        s->pong_score = 0;
        pong_instance_init(s, px, py, pw, ph);
        draw_window();
        return 1;
    }

    // Close
    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01) {
        return -1; // Request close
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Legacy API Wrappers
 * ------------------------------------------------------------------------- */
void pong_init(void) {
    pong_open = 0;
    pong_instance_init(&primary_pong_state, win_x + 30, win_y + 35, win_w - 60, win_h - 60);
}

void pong_open_window(void) {
    pong_open = 1;
    drag = 1;
    maxp_spawn_instance(MAXP_APP_PONG, "Pong Arcade", 0);
}

void pong_close_window(void) {
    pong_open = 0;
    maxp_close_app(MAXP_APP_PONG);
}

void pong_draw(void) {
    pong_instance_draw(&primary_pong_state, win_x + 30, win_y + 35, win_w - 60, win_h - 60);
}

void pong_tick(void) {
    pong_instance_tick(&primary_pong_state, win_x + 30, win_y + 35, win_w - 60, win_h - 60);
}

int pong_handle_click(int mouse_x, int mouse_y) {
    int res = pong_instance_click(&primary_pong_state, win_x + 30, win_y + 35, win_w - 60, win_h - 60, mouse_x, mouse_y);
    if (res == -1) {
        pong_close_window();
        return 1;
    }
    return res;
}

int pong_handle_key(char ascii_char, unsigned char scan_code) {
    int res = pong_instance_key(&primary_pong_state, ascii_char, scan_code);
    if (res == -1) {
        pong_close_window();
        return 1;
    }
    return res;
}

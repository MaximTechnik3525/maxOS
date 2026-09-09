#include "pong.h"
#include "maxp.h"

#include "kernel.h"

int pong_open = 0;

static int pad_x = 0;
static int pad_y = 0;
static int pad_w = 80;
static int pad_h = 12;

static int ball_x = 0;
static int ball_y = 0;
static int ball_dx = 4;
static int ball_dy = 4;
static int ball_size = 8;

static int pong_score = 0;
static int pong_high_score = 0;
static int pong_game_over = 0;

static void draw_ui_btn(int bx, int by, int bw, int bh, const char* label, unsigned short fill, unsigned short text_col) {
    draw_rect(bx, by, bw, bh, 0x0000);
    draw_rect(bx + 1, by + 1, bw - 2, 1, 0xFFFF);
    draw_rect(bx + 1, by + 1, 1, bh - 2, 0xFFFF);
    draw_rect(bx + 2, by + 2, bw - 4, bh - 4, fill);
    print_string((char*)label, bx + 8, by + 5, text_col);
}

void pong_init(void) {
    pong_open = 0;
    pong_score = 0;
    pong_high_score = 0;
    pong_game_over = 0;
}

static void pong_reset_ball(void) {
    int px = win_x + 30;
    int py = win_y + 35;
    int pw = win_w - 60;
    int ph = win_h - 60;

    pad_w = 90;
    pad_h = 12;
    pad_x = px + (pw / 2) - (pad_w / 2);
    pad_y = py + ph - 25;

    ball_x = px + (pw / 2);
    ball_y = py + 70;
    ball_dx = 4;
    ball_dy = 4;
    pong_game_over = 0;
}

void pong_open_window(void) {
    pong_open = 1;
    drag = 1;
    pong_score = 0;
    maxp_set_active_app(MAXP_APP_PONG);
    pong_reset_ball();
    draw_window();
}

void pong_close_window(void) {
    maxp_close_app(MAXP_APP_PONG);
}

void pong_draw(void) {
    prev_cursor();

    int px = win_x + 30;
    int py = win_y + 35;
    int pw = win_w - 60;
    int ph = win_h - 60;

    // Window frame
    draw_rect(px, py, pw, ph, 0x0000);
    draw_rect(px + 1, py + 1, pw - 2, ph - 2, 0xCE79);

    // Titlebar
    draw_rect(px + 2, py + 2, pw - 4, 22, 0x11EB);
    print_string("Pong Arcade - [pong.maxP]", px + 8, py + 6, 0xFFFF);

    // [Close] button
    draw_ui_btn(px + pw - 56, py + 3, 50, 18, "Close", 0xF9A6, 0x0000);

    // Score & Info Bar
    draw_rect(px + 4, py + 26, pw - 8, 22, 0x0000);
    char s_str[16], hs_str[16];
    int_str(pong_score, s_str);
    int_str(pong_high_score, hs_str);

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

    draw_rect(court_x, court_y, court_w, court_h, 0x01A4); // Classic retro dark green

    // Border line inside court
    draw_rect(court_x + 2, court_y + 2, court_w - 4, 2, 0xFFFF);
    draw_rect(court_x + 2, court_y + 2, 2, court_h - 4, 0xFFFF);
    draw_rect(court_x + court_w - 4, court_y + 2, 2, court_h - 4, 0xFFFF);

    if (pong_game_over) {
        print_string("GAME OVER! Press [R] to Play Again", px + (pw / 2) - 150, py + (ph / 2), 0xF800);
    } else {
        // Draw paddle (bright cyan with 3D highlight)
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x0000);
        draw_rect(pad_x + 1, pad_y + 1, pad_w - 2, 2, 0xFFFF);
        draw_rect(pad_x + 1, pad_y + 3, pad_w - 2, pad_h - 4, 0x24EE);

        // Draw ball (bright yellow)
        draw_rect(ball_x, ball_y, ball_size, ball_size, 0xFFE0);
    }

    draw_cursor(pos_x, pos_y);
}

void pong_tick(void) {
    if (!pong_open || pong_game_over) return;

    int px = win_x + 30;
    int py = win_y + 35;
    int pw = win_w - 60;
    int ph = win_h - 60;

    int court_x = px + 6;
    int court_y = py + 52;
    int court_w = pw - 12;
    int court_h = ph - 58;

    // Erase old ball
    draw_rect(ball_x, ball_y, ball_size, ball_size, 0x01A4);

    // Update ball position
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Bounce left wall
    if (ball_x <= court_x + 4) {
        ball_x = court_x + 5;
        ball_dx = -ball_dx;
        play_sound(600); sleep(5); no_sound();
    }
    // Bounce right wall
    if (ball_x + ball_size >= court_x + court_w - 4) {
        ball_x = court_x + court_w - 4 - ball_size;
        ball_dx = -ball_dx;
        play_sound(600); sleep(5); no_sound();
    }
    // Bounce top wall
    if (ball_y <= court_y + 4) {
        ball_y = court_y + 5;
        ball_dy = -ball_dy;
        play_sound(750); sleep(5); no_sound();
    }

    // Paddle collision
    if (ball_y + ball_size >= pad_y && ball_y <= pad_y + pad_h) {
        if (ball_x + ball_size >= pad_x && ball_x <= pad_x + pad_w) {
            ball_y = pad_y - ball_size;
            ball_dy = -ball_dy;
            pong_score += 10;
            if (pong_score > pong_high_score) pong_high_score = pong_score;
            play_sound(950); sleep(8); no_sound();

            // Refresh score header
            draw_rect(px + 4, py + 26, pw - 8, 22, 0x0000);
            char s_str[16], hs_str[16];
            int_str(pong_score, s_str);
            int_str(pong_high_score, hs_str);
            print_string("Score: ", px + 12, py + 32, 0x07E0);
            print_string(s_str, px + 68, py + 32, 0xFFFF);
            print_string("High: ", px + 140, py + 32, 0xFCEF);
            print_string(hs_str, px + 190, py + 32, 0xFFFF);
            print_string("Controls: [A] Left  [D] Right  [R] Restart  [C] Exit", px + 270, py + 32, 0x24EE);
        }
    }

    // Missed paddle: Game Over
    if (ball_y >= court_y + court_h) {
        pong_game_over = 1;
        play_sound(200); sleep(100); play_sound(150); sleep(120); no_sound();
        pong_draw();
        return;
    }

    // Redraw ball
    draw_rect(ball_x, ball_y, ball_size, ball_size, 0xFFE0);
}

int pong_handle_click(int mouse_x, int mouse_y) {
    if (!pong_open) return 0;

    int px = win_x + 30;
    int py = win_y + 35;
    int pw = win_w - 60;
    int ph = win_h - 60;

    // [Close] button
    if (mouse_x >= px + pw - 56 && mouse_x <= px + pw - 6 && mouse_y >= py + 3 && mouse_y <= py + 21) {
        pong_close_window();
        return 1;
    }

    // Move paddle toward mouse if clicked inside court
    int court_x = px + 6;
    int court_w = pw - 12;
    if (mouse_x >= court_x && mouse_x <= court_x + court_w) {
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x01A4);
        pad_x = mouse_x - (pad_w / 2);
        if (pad_x < court_x + 4) pad_x = court_x + 4;
        if (pad_x + pad_w > court_x + court_w - 4) pad_x = court_x + court_w - 4 - pad_w;
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x0000);
        draw_rect(pad_x + 1, pad_y + 1, pad_w - 2, 2, 0xFFFF);
        draw_rect(pad_x + 1, pad_y + 3, pad_w - 2, pad_h - 4, 0x24EE);
    }

    // If game over, click restarts
    if (pong_game_over) {
        pong_score = 0;
        pong_reset_ball();
        pong_draw();
        return 1;
    }

    if (mouse_x >= px && mouse_x <= px + pw && mouse_y >= py && mouse_y <= py + ph) {
        return 1;
    }

    return 0;
}

int pong_handle_key(char ascii_char, unsigned char scan_code) {
    if (!pong_open) return 0;

    int px = win_x + 30;
    int pw = win_w - 60;
    int court_x = px + 6;
    int court_w = pw - 12;

    // Left: 'a', 'A', 'L', or left arrow
    if (ascii_char == 'a' || ascii_char == 'A' || ascii_char == 'L' || scan_code == 0x4B) {
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x01A4);
        pad_x -= 30;
        if (pad_x < court_x + 4) pad_x = court_x + 4;
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x0000);
        draw_rect(pad_x + 1, pad_y + 1, pad_w - 2, 2, 0xFFFF);
        draw_rect(pad_x + 1, pad_y + 3, pad_w - 2, pad_h - 4, 0x24EE);
        return 1;
    }

    // Right: 'd', 'D', 'R', or right arrow
    if (ascii_char == 'd' || ascii_char == 'D' || ascii_char == 'R' || scan_code == 0x4D) {
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x01A4);
        pad_x += 30;
        if (pad_x + pad_w > court_x + court_w - 4) pad_x = court_x + court_w - 4 - pad_w;
        draw_rect(pad_x, pad_y, pad_w, pad_h, 0x0000);
        draw_rect(pad_x + 1, pad_y + 1, pad_w - 2, 2, 0xFFFF);
        draw_rect(pad_x + 1, pad_y + 3, pad_w - 2, pad_h - 4, 0x24EE);
        return 1;
    }

    // Restart: 'r' or 'R'
    if (ascii_char == 'r' || ascii_char == 'R') {
        pong_score = 0;
        pong_reset_ball();
        pong_draw();
        return 1;
    }

    // Close: 'c', 'q', Esc
    if (ascii_char == 'c' || ascii_char == 'C' || ascii_char == 'q' || scan_code == 0x01) {
        pong_close_window();
        return 1;
    }

    return 0;
}

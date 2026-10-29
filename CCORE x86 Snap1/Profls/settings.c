#include "../System/DWM.h"

extern Window windows[8];
extern int window_count;
extern MouseState mouse;
extern SystemSettings sys_settings;

static int current_tab = 0; // 0: Personalization, 1: System

void render_settings_content(Window* self) {
    // Sol Sekme Menüsü
    int p_tab_pressed = (current_tab == 0);
    draw_3d_box(self->x + 10, self->y + 25, 130, 22, p_tab_pressed ? 1 : 0);
    draw_string(self->x + 15, self->y + 32, "PERSONALIZATION", 0x00);

    int s_tab_pressed = (current_tab == 1);
    draw_3d_box(self->x + 10, self->y + 52, 120, 22, s_tab_pressed ? 1 : 0);
    draw_string(self->x + 15, self->y + 59, "SYSTEM CONFIG", 0x00);

    // Sağ İçerik Paneli
    draw_3d_box(self->x + 135, self->y + 25, self->w - 145, self->h - 35, 1);

    if (current_tab == 0) { // Personalization
        draw_string(self->x + 145, self->y + 35, "DESKTOP BACKGROUND COLOR:", 0x00);
        unsigned char colors[4] = {0x01, 0x3B, 0x2C, 0x07};

        for (int i = 0; i < 4; i++) {
            int bx = self->x + 145 + (i * 35);
            int by = self->y + 50;

            int pressed = (
                mouse.left_btn &&
                mouse.x >= bx &&
                mouse.x <= bx + 28 &&
                mouse.y >= by &&
                mouse.y <= by + 20
            );

            draw_3d_box(bx, by, 28, 20, pressed);
            fill_rect(bx + 3, by + 3, 22, 14, colors[i]);
        }

        draw_string(self->x + 145, self->y + 85, "BACKGROUND PATTERN:", 0x00);

        int p_pressed = (
            mouse.left_btn &&
            mouse.x >= self->x + 145 &&
            mouse.x <= self->x + 245 &&
            mouse.y >= self->y + 100 &&
            mouse.y <= self->y + 120
        );

        draw_3d_box(self->x + 145, self->y + 100, 100, 20, p_pressed);

        draw_string(
            self->x + 155 + (p_pressed ? 1 : 0),
            self->y + 106 + (p_pressed ? 1 : 0),
            "TOGGLE DITHER",
            0x00
        );
    } else { // System Config
        draw_string(self->x + 145, self->y + 35, "SCREEN RESOLUTION:", 0x00);

        int res0_p = (sys_settings.resolution_mode == 0);
        draw_3d_box(self->x + 145, self->y + 50, 100, 20, res0_p);
        draw_string(self->x + 150, self->y + 56, "640x480 VBE", 0x00);

        int res1_p = (sys_settings.resolution_mode == 1);
        draw_3d_box(self->x + 250, self->y + 50, 100, 20, res1_p);
        draw_string(self->x + 255, self->y + 56, "800x600 VBE", 0x00);

        draw_string(self->x + 145, self->y + 85, "MAX SYSTEM RAM LIMIT:", 0x00);

        int ram_opts[4] = {16, 32, 64, 128};

        for (int i = 0; i < 4; i++) {
            int bx = self->x + 145 + (i * 52);
            int by = self->y + 100;
            int selected = (sys_settings.ram_limit_mb == ram_opts[i]);

            draw_3d_box(bx, by, 48, 20, selected ? 1 : 0);

            char r_str[8];

            r_str[0] = '0' + (ram_opts[i] / 100);
            r_str[1] = '0' + ((ram_opts[i] / 10) % 10);
            r_str[2] = '0' + (ram_opts[i] % 10);
            r_str[3] = '\0';

            if (r_str[0] == '0') {
                r_str[0] = r_str[1];
                r_str[1] = r_str[2];
                r_str[2] = '\0';
            }

            draw_string(bx + 6, by + 6, r_str, 0x00);
        }
    }
}

void click_settings_content(Window* self, int mx, int my) {
    if (
        mx >= self->x + 10 &&
        mx <= self->x + 140 &&
        my >= self->y + 25 &&
        my <= self->y + 47
    ) {
        current_tab = 0;
        return;
    }

    if (
        mx >= self->x + 10 &&
        mx <= self->x + 130 &&
        my >= self->y + 52 &&
        my <= self->y + 74
    ) {
        current_tab = 1;
        return;
    }

    if (current_tab == 0) {
        unsigned char colors[4] = {0x01, 0x3B, 0x2C, 0x07};

        for (int i = 0; i < 4; i++) {
            int bx = self->x + 145 + (i * 35);
            int by = self->y + 50;

            if (
                mx >= bx &&
                mx <= bx + 28 &&
                my >= by &&
                my <= by + 20
            ) {
                sys_settings.bg_color = colors[i];
                return;
            }
        }

        if (
            mx >= self->x + 145 &&
            mx <= self->x + 245 &&
            my >= self->y + 100 &&
            my <= self->y + 120
        ) {
            sys_settings.bg_pattern_enabled = !sys_settings.bg_pattern_enabled;
        }
    } else {
        if (
            mx >= self->x + 145 &&
            mx <= self->x + 245 &&
            my >= self->y + 50 &&
            my <= self->y + 70
        ) {
            sys_settings.resolution_mode = 0;
        }

        if (
            mx >= self->x + 250 &&
            mx <= self->x + 350 &&
            my >= self->y + 50 &&
            my <= self->y + 70
        ) {
            sys_settings.resolution_mode = 1;
        }

        int ram_opts[4] = {16, 32, 64, 128};

        for (int i = 0; i < 4; i++) {
            int bx = self->x + 145 + (i * 52);
            int by = self->y + 100;

            if (
                mx >= bx &&
                mx <= bx + 48 &&
                my >= by &&
                my <= by + 20
            ) {
                sys_settings.ram_limit_mb = ram_opts[i];
                return;
            }
        }
    }
}

void launch_settings_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (
            windows[i].title[0] == 'S' &&
            windows[i].title[1] == 'E'
        ) {
            windows[i].is_visible = 1;
            return;
        }
    }

    if (window_count < 8) {
        Window* w = &windows[window_count++];

        w->id = 2;
        w->x = 60;
        w->y = 40;
        w->w = 380;
        w->h = 220;

        w->title[0] = 'S';
        w->title[1] = 'E';
        w->title[2] = 'T';
        w->title[3] = 'T';
        w->title[4] = 'I';
        w->title[5] = 'N';
        w->title[6] = 'G';
        w->title[7] = 'S';
        w->title[8] = '\0';

        w->is_visible = 1;
        w->render_content = render_settings_content;
        w->click_event = click_settings_content;
    }
}
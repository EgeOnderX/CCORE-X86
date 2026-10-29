#include "../System/DWM.h"

extern Window windows[8];
extern int window_count;
extern MouseState mouse;
extern unsigned int system_ticks;

void render_taskman_content(Window* self) {
    draw_string(self->x + 10, self->y + 24, "PROCESS MANAGER:", 0x00);
    draw_3d_box(self->x + 8, self->y + 36, self->w - 16, 75, 1);

    int row_y = self->y + 40;
    for (int i = 0; i < window_count; i++) {
        if (!windows[i].is_visible) continue;

        draw_string(self->x + 12, row_y, windows[i].title, 0x00);
        draw_string(self->x + 100, row_y, "PID:12", 0x08);

        int kill_x = self->x + self->w - 60;
        int pressed = (mouse.left_btn && mouse.x >= kill_x && mouse.x <= kill_x + 45 && mouse.y >= row_y - 2 && mouse.y <= row_y + 12);
        draw_3d_box(kill_x, row_y - 2, 45, 14, pressed);
        draw_string(kill_x + 5 + (pressed?1:0), row_y + (pressed?1:0), "KILL", 0x00);

        row_y += 16;
    }

    // Gerçek Sistem Uptime ve CPU Ticks Göstergesi
    draw_string(self->x + 10, self->y + 118, "TICKS:", 0x00);
    char tick_buf[10];
    tick_buf[0] = '0' + ((system_ticks / 10000) % 10);
    tick_buf[1] = '0' + ((system_ticks / 1000) % 10);
    tick_buf[2] = '0' + ((system_ticks / 100) % 10);
    tick_buf[3] = '0' + ((system_ticks / 10) % 10);
    tick_buf[4] = '0' + (system_ticks % 10);
    tick_buf[5] = '\0';
    draw_string(self->x + 60, self->y + 118, tick_buf, 0x01);

    draw_string(self->x + 10, self->y + 132, "RAM: 64MB / ALLOC: 12MB", 0x00);
}

void click_taskman_content(Window* self, int mx, int my) {
    int row_y = self->y + 40;
    for (int i = 0; i < window_count; i++) {
        if (!windows[i].is_visible) continue;

        int kill_x = self->x + self->w - 60;
        if (mx >= kill_x && mx <= kill_x + 45 && my >= row_y - 2 && my <= row_y + 12) {
            windows[i].is_visible = 0;
            return;
        }
        row_y += 16;
    }
}

void launch_taskman_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'T' && windows[i].title[1] == 'A') { windows[i].is_visible = 1; return; }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 4; w->x = 120; w->y = 70; w->w = 220; w->h = 150;
        w->title[0] = 'T'; w->title[1] = 'A'; w->title[2] = 'S'; w->title[3] = 'K'; w->title[4] = 'S'; w->title[5] = '\0';
        w->is_visible = 1; w->render_content = render_taskman_content; w->click_event = click_taskman_content;
    }
}
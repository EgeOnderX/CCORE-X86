#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;
extern MouseState mouse;

void render_filemgr_content(Window* self) {
    draw_string(self->x + 10, self->y + 25, "C:\\ Virtual Sector Files:", 0x00);
    draw_3d_box(self->x + 8, self->y + 38, self->w - 16, self->h - 48, 1);

    draw_string(self->x + 16, self->y + 48, "NOTEPAD.DAT   (Sector 20)", 0x00);
    draw_string(self->x + 16, self->y + 66, "PAINT.DAT     (Sector 30)", 0x00);
    draw_string(self->x + 16, self->y + 84, "CONFIG.DAT    (Sector 10)", 0x00);
    draw_string(self->x + 16, self->y + 102, "BOOT.BIN     (Sector 1-48)", 0x00);
}

void click_filemgr_content(Window* self, int mx, int my) {
}

void launch_filemgr_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'F' && windows[i].title[1] == 'I') {
            windows[i].is_visible = 1;
            return;
        }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 3; w->x = 120; w->y = 60; w->w = 260; w->h = 180;
        w->title[0] = 'F'; w->title[1] = 'I'; w->title[2] = 'L'; w->title[3] = 'E'; w->title[4] = 'M'; w->title[5] = 'G'; w->title[6] = 'R'; w->title[7] = '\0';
        w->is_visible = 1;
        w->render_content = render_filemgr_content;
        w->click_event = click_filemgr_content;
    }
}
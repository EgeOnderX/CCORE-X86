#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;

void render_filemgr_content(Window* self) {
    draw_3d_box(self->x + 8, self->y + 22, self->w - 16, 16, 1);
    draw_string(self->x + 12, self->y + 26, "A:\\PROFLS\\*", 0x00);

    draw_string(self->x + 12, self->y + 45, "BOOT.SYS      512B  SYS", 0x00);
    draw_string(self->x + 12, self->y + 60, "KERNEL.BIN    12KB  BIN", 0x00);
    draw_string(self->x + 12, self->y + 75, "CALC.C         4KB  SRC", 0x00);
    draw_string(self->x + 12, self->y + 90, "PAINT.C        4KB  SRC", 0x00);
    draw_string(self->x + 12, self->y + 105, "SETTINGS.C    3KB  SRC", 0x00);
    draw_string(self->x + 12, self->y + 120, "TASKMAN.C     3KB  SRC", 0x00);
}

void launch_filemgr_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'F' && windows[i].title[1] == 'I') { windows[i].is_visible = 1; return; }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 3; w->x = 140; w->y = 50; w->w = 210; w->h = 145;
        w->title[0] = 'F'; w->title[1] = 'I'; w->title[2] = 'L'; w->title[3] = 'E'; w->title[4] = 'S'; w->title[5] = '\0';
        w->is_visible = 1; w->render_content = render_filemgr_content; w->click_event = 0;
    }
}
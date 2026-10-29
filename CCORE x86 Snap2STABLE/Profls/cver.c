#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;

void render_cver_content(Window* self) {
    draw_3d_box(self->x + 10, self->y + 24, self->w - 20, 110, 1);

    draw_string(self->x + 20, self->y + 32, "CCORE x86 V1.0 OS", 0x01);
    draw_string(self->x + 20, self->y + 48, "Build: 2026.10-PROT", 0x00);
    draw_string(self->x + 20, self->y + 64, "Arch: x86 32-Bit PM", 0x00);
    draw_string(self->x + 20, self->y + 80, "VGA: VBE 640x480 LFB", 0x00);
    draw_string(self->x + 20, self->y + 96, "Developer: Ege Onder", 0x00);
}

void launch_cver_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'A' && windows[i].title[1] == 'B') { windows[i].is_visible = 1; return; }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 5; w->x = 140; w->y = 70; w->w = 210; w->h = 145;
        w->title[0] = 'A'; w->title[1] = 'B'; w->title[2] = 'O'; w->title[3] = 'U'; w->title[4] = 'T'; w->title[5] = '\0';
        w->is_visible = 1; w->render_content = render_cver_content; w->click_event = 0;
    }
}
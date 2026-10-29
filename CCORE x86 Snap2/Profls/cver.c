#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;

void render_cver_content(Window* w) {
    int rx = w->x;
    int ry = w->y + 20;

    fill_rect(rx + 3, ry + 3, w->w - 6, w->h - 23, 0x07);
    
    draw_string(rx + 20, ry + 25, "CCORE Operating System", 0x0F);
    draw_string(rx + 20, ry + 45, "Version 1.0.0 x86 Bare-Metal", 0x00);
    draw_string(rx + 20, ry + 65, "Architecture: 32-bit Protected Mode", 0x00);
    draw_string(rx + 20, ry + 85, "Submenu & ATA Disk IO Supported", 0x00);
    draw_string(rx + 20, ry + 115, "Copyright (C) 2026 Ege Onder", 0x01);
}

void launch_cver_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].is_visible && windows[i].title[0] == 'A' && windows[i].title[1] == 'b') {
            return;
        }
    }

    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 5;
        w->x = 180;
        w->y = 140;
        w->w = 300;
        w->h = 160;
        w->is_visible = 1;
        w->x_btn_pressed = 0;
        w->about_btn_pressed = 0;
        
        char* t = "About CCORE";
        for(int i=0; i<12; i++) w->title[i] = t[i];

        w->render_content = render_cver_content;
        w->click_event = 0;
    }
}
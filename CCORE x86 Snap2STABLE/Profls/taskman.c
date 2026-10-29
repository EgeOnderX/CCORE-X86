#include "../System/DWM.h"

extern Window windows[8];
extern int window_count;
extern MouseState mouse;
extern unsigned int system_ticks;

void render_taskman_content(Window* self) {
    draw_string(self->x + 10, self->y + 22, "Active Processes:", 0x00);
    draw_3d_box(self->x + 8, self->y + 32, self->w - 16, 72, 1);

    int row_y = self->y + 37;
    int active_tasks = 0;
    
    for (int i = 0; i < 8; i++) {
        if (!windows[i].is_visible || windows[i].title[0] == '\0') continue;
        active_tasks++;

        // Süreç Adı (Sol Sütun)
        draw_string(self->x + 14, row_y, windows[i].title, 0x00);
        
        // PID (Orta Sütun)
        draw_string(self->x + 100, row_y, "PID:", 0x08);
        char pid_buf[4] = {'0' + (i + 1), '\0'};
        draw_string(self->x + 128, row_y, pid_buf, 0x08);

        // Kill Butonu (Sağ Sütun)
        int kill_x = self->x + self->w - 56;
        int pressed = (mouse.left_btn && mouse.x >= kill_x && mouse.x <= kill_x + 42 && mouse.y >= row_y - 1 && mouse.y <= row_y + 11);
        draw_3d_box(kill_x, row_y - 1, 42, 12, pressed);
        draw_string(kill_x + 8 + (pressed?1:0), row_y + (pressed?1:0), "Kill", 0x00);

        row_y += 15;
    }

    if (active_tasks == 0) {
        draw_string(self->x + 14, row_y + 4, "No active processes.", 0x08);
    }

    // Alt Performans ve Uptime Paneli
    int usage_y = self->y + 112;
    
    unsigned int uptime_sec = system_ticks / 50;
    draw_string(self->x + 10, usage_y, "Up:", 0x00);
    char up_buf[10];
    int val = uptime_sec;
    int idx = 0;
    if (val == 0) {
        up_buf[idx++] = '0';
    } else {
        char temp[8];
        int ti = 0;
        while (val > 0 && ti < 7) {
            temp[ti++] = '0' + (val % 10);
            val /= 10;
        }
        while (ti > 0) {
            up_buf[idx++] = temp[--ti];
        }
    }
    up_buf[idx++] = 's';
    up_buf[idx] = '\0';
    draw_string(self->x + 38, usage_y, up_buf, 0x01);

    draw_string(self->x + 105, usage_y, "CPU:", 0x00);
    int cpu = (system_ticks % 25) + 2;
    char cpu_buf[6] = {'0' + (cpu/10), '0' + (cpu%10), '%', '\0'};
    draw_string(self->x + 138, usage_y, cpu_buf, 0x02);

    draw_string(self->x + 10, usage_y + 14, "RAM: 32MB / Heap: Stable", 0x00);
}

void click_taskman_content(Window* self, int mx, int my) {
    int row_y = self->y + 37;
    for (int i = 0; i < 8; i++) {
        if (!windows[i].is_visible || windows[i].title[0] == '\0') continue;

        int kill_x = self->x + self->w - 56;
        if (mx >= kill_x && mx <= kill_x + 42 && my >= row_y - 1 && my <= row_y + 11) {
            windows[i].is_visible = 0;
            windows[i].title[0] = '\0';
            return;
        }
        row_y += 15;
    }
}

void launch_taskman_app(void) {
    for (int i = 0; i < 8; i++) {
        if (windows[i].is_visible && windows[i].title[0] == 'T' && windows[i].title[1] == 'a') { 
            return; 
        }
    }
    
    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!windows[i].is_visible || windows[i].title[0] == '\0') {
            slot = i;
            break;
        }
    }
    
    if (slot != -1) {
        Window* w = &windows[slot];
        w->id = 4; w->x = 100; w->y = 50; w->w = 260; w->h = 150;
        w->title[0] = 'T'; w->title[1] = 'a'; w->title[2] = 's'; w->title[3] = 'k'; w->title[4] = 's'; w->title[5] = '\0';
        w->is_visible = 1; 
        w->render_content = render_taskman_content; 
        w->click_event = click_taskman_content;
        if (slot >= window_count) {
            window_count = slot + 1;
        }
    }
}
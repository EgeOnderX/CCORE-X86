#include "../System/DWM.h"

extern Window windows[8];
extern int window_count;
extern MouseState mouse;
extern SystemSettings sys_settings;

static int current_tab = 0; // 0: Personalization, 1: System

void render_settings_content(Window* self) {
    // Sol Sekme Menüsü (Genişlik ve Hizalama Düzeltildi)
    int p_tab_pressed = (current_tab == 0);
    draw_3d_box(self->x + 10, self->y + 25, 125, 22, p_tab_pressed ? 1 : 0);
    draw_string(self->x + 14, self->y + 32, "Personalization", 0x00);

    int s_tab_pressed = (current_tab == 1);
    draw_3d_box(self->x + 10, self->y + 52, 125, 22, s_tab_pressed ? 1 : 0);
    draw_string(self->x + 14, self->y + 59, "System Config", 0x00);

    // Sağ İçerik Paneli (Taşma Yapmayacak Şekilde Genişletildi)
    draw_3d_box(self->x + 140, self->y + 25, self->w - 150, self->h - 35, 1);

    if (current_tab == 0) { // Personalization & Mouse Settings
        draw_string(self->x + 150, self->y + 33, "Desktop Background Color:", 0x00);
        unsigned char colors[4] = {0x01, 0x3B, 0x2C, 0x07};

        for (int i = 0; i < 4; i++) {
            int bx = self->x + 150 + (i * 42);
            int by = self->y + 46;
            int selected = (sys_settings.bg_color == colors[i]);

            draw_3d_box(bx, by, 36, 18, selected ? 1 : 0);
            fill_rect(bx + 3, by + 3, 30, 12, colors[i]);
        }

        draw_string(self->x + 150, self->y + 72, "Background Pattern:", 0x00);
        int p_pressed = sys_settings.bg_pattern_enabled;
        draw_3d_box(self->x + 150, self->y + 84, 110, 18, p_pressed);
        draw_string(self->x + 160, self->y + 89, p_pressed ? "Pattern: ON" : "Pattern: OFF", 0x00);

        // Yeni Detaylı Fare (Mouse) Kişiselleştirme Seçenekleri
        draw_string(self->x + 150, self->y + 110, "Mouse Speed:", 0x00);
        int speeds[3] = {1, 2, 3};
        const char* speed_labels[3] = {"1x", "2x", "3x"};
        for (int i = 0; i < 3; i++) {
            int bx = self->x + 150 + (i * 45);
            int by = self->y + 122;
            int sel = (sys_settings.mouse_speed == speeds[i]);
            draw_3d_box(bx, by, 40, 18, sel ? 1 : 0);
            draw_string(bx + 12, by + 5, speed_labels[i], 0x00);
        }

        draw_string(self->x + 150, self->y + 148, "Mouse Trails:", 0x00);
        int t_pressed = sys_settings.mouse_trails;
        draw_3d_box(self->x + 150, self->y + 160, 110, 18, t_pressed);
        draw_string(self->x + 160, self->y + 165, t_pressed ? "Trails: ON" : "Trails: OFF", 0x00);

    } else { // System Config
        draw_string(self->x + 150, self->y + 35, "Screen Resolution:", 0x00);

        int res0_p = (sys_settings.resolution_mode == 0);
        draw_3d_box(self->x + 150, self->y + 50, 105, 20, res0_p);
        draw_string(self->x + 158, self->y + 56, "640x480 VBE", 0x00);

        int res1_p = (sys_settings.resolution_mode == 1);
        draw_3d_box(self->x + 265, self->y + 50, 105, 20, res1_p);
        draw_string(self->x + 273, self->y + 56, "800x600 VBE", 0x00);

        draw_string(self->x + 150, self->y + 88, "Max System RAM Limit:", 0x00);

        int ram_opts[4] = {16, 32, 64, 128};
        for (int i = 0; i < 4; i++) {
            int bx = self->x + 150 + (i * 55);
            int by = self->y + 102;
            int selected = (sys_settings.ram_limit_mb == ram_opts[i]);

            draw_3d_box(bx, by, 50, 20, selected ? 1 : 0);
            
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
            draw_string(bx + 12, by + 6, r_str, 0x00);
        }
    }
}

void click_settings_content(Window* self, int mx, int my) {
    // Sekme Değiştirme Tıklamaları
    if (mx >= self->x + 10 && mx <= self->x + 135 && my >= self->y + 25 && my <= self->y + 47) {
        current_tab = 0;
        return;
    }

    if (mx >= self->x + 10 && mx <= self->x + 135 && my >= self->y + 52 && my <= self->y + 74) {
        current_tab = 1;
        return;
    }

    if (current_tab == 0) {
        // Arka Plan Renk Seçimi
        unsigned char colors[4] = {0x01, 0x3B, 0x2C, 0x07};
        for (int i = 0; i < 4; i++) {
            int bx = self->x + 150 + (i * 42);
            int by = self->y + 46;
            if (mx >= bx && mx <= bx + 36 && my >= by && my <= by + 18) {
                sys_settings.bg_color = colors[i];
                return;
            }
        }

        // Desen Aç/Kapat
        if (mx >= self->x + 150 && mx <= self->x + 260 && my >= self->y + 84 && my <= self->y + 102) {
            sys_settings.bg_pattern_enabled = !sys_settings.bg_pattern_enabled;
            return;
        }

        // Fare Hızı Seçimi
        int speeds[3] = {1, 2, 3};
        for (int i = 0; i < 3; i++) {
            int bx = self->x + 150 + (i * 45);
            int by = self->y + 122;
            if (mx >= bx && mx <= bx + 40 && my >= by && my <= by + 18) {
                sys_settings.mouse_speed = speeds[i];
                return;
            }
        }

        // Fare İzleri Aç/Kapat
        if (mx >= self->x + 150 && mx <= self->x + 260 && my >= self->y + 160 && my <= self->y + 178) {
            sys_settings.mouse_trails = !sys_settings.mouse_trails;
            return;
        }

    } else {
        // Çözünürlük Seçimleri
        if (mx >= self->x + 150 && mx <= self->x + 255 && my >= self->y + 50 && my <= self->y + 70) {
            sys_settings.resolution_mode = 0;
            return;
        }

        if (mx >= self->x + 265 && mx <= self->x + 370 && my >= self->y + 50 && my <= self->y + 70) {
            sys_settings.resolution_mode = 1;
            return;
        }

        // RAM Limiti Seçimleri
        int ram_opts[4] = {16, 32, 64, 128};
        for (int i = 0; i < 4; i++) {
            int bx = self->x + 150 + (i * 55);
            int by = self->y + 102;
            if (mx >= bx && mx <= bx + 50 && my >= by && my <= by + 20) {
                sys_settings.ram_limit_mb = ram_opts[i];
                return;
            }
        }
    }
}

void launch_settings_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'S' && windows[i].title[1] == 'E') {
            windows[i].is_visible = 1;
            return;
        }
    }

    if (window_count < 8) {
        Window* w = &windows[window_count++];

        w->id = 2;
        w->x = 50;
        w->y = 40;
        w->w = 400; // Genişletildi
        w->h = 240; // Yükseklik artırıldı

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
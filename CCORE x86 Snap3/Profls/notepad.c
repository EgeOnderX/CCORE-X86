#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;
extern MouseState mouse;

static char notepad_text[256] = "Welcome to CCORE Notepad.\nType your notes here...";
static int notepad_cursor_pos = 46;

void render_notepad_content(Window* self) {
    // Üst Araç Çubuğu (Save Butonu)
    draw_3d_box(self->x + 8, self->y + 22, 50, 16, 0);
    draw_string(self->x + 14, self->y + 26, "SAVE", 0x00);

    // Metin Alanı Kutusu
    draw_3d_box(self->x + 8, self->y + 42, self->w - 16, self->h - 52, 1);

    // Metni ekrana satır satır yazdır
    int rx = self->x + 14;
    int ry = self->y + 50;
    char line_buf[40];
    int l_idx = 0;

    for (int i = 0; notepad_text[i] != '\0' && i < 255; i++) {
        if (notepad_text[i] == '\n' || l_idx >= 38) {
            line_buf[l_idx] = '\0';
            draw_string(rx, ry, line_buf, 0x00);
            ry += 12;
            l_idx = 0;
            if (notepad_text[i] != '\n') line_buf[l_idx++] = notepad_text[i];
        } else {
            line_buf[l_idx++] = notepad_text[i];
        }
    }
    line_buf[l_idx] = '\0';
    draw_string(rx, ry, line_buf, 0x00);
}

void click_notepad_content(Window* self, int mx, int my) {
    // Save Butonu Tıklaması -> Gerçek Disk Yazma (Sector 20)
    if (mx >= self->x + 8 && mx <= self->x + 58 && my >= self->y + 22 && my <= self->y + 38) {
        // Notepad içeriğini ATA disk üzerinden sektöre kaydet
        ata_write_sector(20, (unsigned char*)notepad_text);
        draw_string(self->x + 65, self->y + 26, "SAVED!", 0x0A);
        return;
    }
}

void launch_notepad_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'N' && windows[i].title[1] == 'O') {
            windows[i].is_visible = 1;
            return;
        }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 7; w->x = 140; w->y = 70; w->w = 320; w->h = 200;
        char* t = "NOTEPAD";
        for(int i=0; i<8; i++) w->title[i] = t[i];
        w->is_visible = 1;
        w->render_content = render_notepad_content;
        w->click_event = click_notepad_content;
    }
}
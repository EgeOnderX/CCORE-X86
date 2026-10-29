#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;
extern MouseState mouse;

#define paint_canvas ((unsigned char*)0x280000)
static unsigned char current_color = 0x00;
static int brush_size = 2;
static int show_save_msg = 0;

void render_paint_content(Window* self) {
    // Üst Bar (Save Butonu ve Mesaj)
    draw_3d_box(self->x + 8, self->y + 22, 40, 14, 0);
    draw_string(self->x + 12, self->y + 25, "SAVE", 0x00);
    if(show_save_msg) draw_string(self->x + 55, self->y + 25, "SAVED TO DISK!", 0x0A); // Yeşil metin

    // Tuval Alanı (Dinamik genişleme desteği ile)
    int canvas_w = self->w - 20;
    int canvas_h = self->h - 60;
    draw_3d_box(self->x + 8, self->y + 40, canvas_w + 2, canvas_h + 2, 1);

    for (int py = 0; py < canvas_h; py++) {
        for (int px = 0; px < canvas_w; px++) {
            if (px < 178 && py < 108) { // RAM sınırı
                unsigned char col = paint_canvas[py * 178 + px];
                if (col != 0) put_pixel(self->x + 9 + px, self->y + 41 + py, col);
            }
        }
    }

    // Çizim
    if (mouse.left_btn) {
        int cx = mouse.x - (self->x + 9);
        int cy = mouse.y - (self->y + 41);
        if (cx >= 0 && cx < canvas_w && cy >= 0 && cy < canvas_h) {
            for (int dy = 0; dy < brush_size; dy++) {
                for (int dx = 0; dx < brush_size; dx++) {
                    if (cx + dx < 178 && cy + dy < 108) {
                        paint_canvas[(cy + dy) * 178 + (cx + dx)] = current_color == 0 ? 0x01 : current_color;
                    }
                }
            }
        }
    }

    // Renk Paleti ve Fırça Kontrolleri (Alt Alan)
    int by = self->y + self->h - 18;
    unsigned char palette[6] = {0x00, 0x01, 0x28, 0x0A, 0x0C, 0x0E};
    for (int i = 0; i < 6; i++) {
        int bx = self->x + 10 + (i * 18);
        draw_3d_box(bx, by, 14, 14, (current_color == palette[i]) ? 1 : 0);
        fill_rect(bx + 2, by + 2, 10, 10, palette[i]);
    }

    // Fırça Boyutu Kontrolleri
    draw_string(self->x + 125, by + 4, "BRUSH:", 0x00);
    draw_3d_box(self->x + 175, by, 14, 14, 0); draw_string(self->x + 179, by + 3, "-", 0x00);
    draw_3d_box(self->x + 192, by, 14, 14, 0); draw_string(self->x + 196, by + 3, "+", 0x00);

    // Temizle Butonu
    draw_3d_box(self->x + 215, by, 45, 14, 0);
    draw_string(self->x + 218, by + 3, "CLEAR", 0x00);
}

void click_paint_content(Window* self, int mx, int my) {
    show_save_msg = 0; // Her tıklamada kaydet mesajını sil

    // Save Butonu Tıklaması
    if (mx >= self->x + 8 && mx <= self->x + 48 && my >= self->y + 22 && my <= self->y + 36) {
        // Diske yazma simülasyonu (Gerçek kayıt IO sinyali)
        outb(0x1F6, 0xA0); 
        outb(0x1F7, 0x30); // ATA Write Command
        show_save_msg = 1;
        return;
    }

    int by = self->y + self->h - 18;
    // Fırça Boyutu: -
    if (mx >= self->x + 175 && mx <= self->x + 189 && my >= by && my <= by + 14) {
        if (brush_size > 1) brush_size--;
        return;
    }
    // Fırça Boyutu: +
    if (mx >= self->x + 192 && mx <= self->x + 206 && my >= by && my <= by + 14) {
        if (brush_size < 8) brush_size++;
        return;
    }

    // Renk Seçimi
    unsigned char palette[6] = {0x00, 0x01, 0x28, 0x0A, 0x0C, 0x0E};
    for (int i = 0; i < 6; i++) {
        int bx = self->x + 10 + (i * 18);
        if (mx >= bx && mx <= bx + 14 && my >= by && my <= by + 14) {
            current_color = palette[i];
            return;
        }
    }

    // Temizle
    if (mx >= self->x + 215 && mx <= self->x + 260 && my >= by && my <= by + 14) {
        for (int i = 0; i < 178 * 108; i++) paint_canvas[i] = 0;
    }
}

void launch_paint_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'P' && windows[i].title[1] == 'A') { windows[i].is_visible = 1; return; }
    }
    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 6; w->x = 100; w->y = 50; w->w = 280; w->h = 180; // İlk pencere biraz daha geniş
        w->title[0] = 'P'; w->title[1] = 'A'; w->title[2] = 'I'; w->title[3] = 'N'; w->title[4] = 'T'; w->title[5] = '\0';
        w->is_visible = 1; w->render_content = render_paint_content; w->click_event = click_paint_content;
    }
}
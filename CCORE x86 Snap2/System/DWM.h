#ifndef KERNEL_H
#define KERNEL_H

#define SCREEN_W 640
#define SCREEN_H 480

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define MOUSE_TRAIL_MAX 8

typedef struct {
    int x;
    int y;
    unsigned char left_btn;
    unsigned char right_btn;
    unsigned char left_clicked;
    unsigned char left_released;
    int trail_x[MOUSE_TRAIL_MAX];
    int trail_y[MOUSE_TRAIL_MAX];
} MouseState;

typedef struct Window {
    int id;
    int x, y;
    int w, h;
    char title[32];
    int is_visible;
    int is_dragging;
    int drag_off_x, drag_off_y;
    unsigned char x_btn_pressed;
    unsigned char about_btn_pressed;
    void (*render_content)(struct Window* self);
    void (*click_event)(struct Window* self, int mx, int my);
} Window;

typedef struct {
    unsigned char bg_color;
    int bg_pattern_enabled;
    int resolution_mode;
    int ram_limit_mb;
    int mouse_speed;
    int mouse_trails;
    int mouse_size;
} SystemSettings;

void put_pixel(int x, int y, unsigned char color);
void flip_buffer(void);
void fill_rect(int x, int y, int w, int h, unsigned char color);
void draw_3d_box(int x, int y, int w, int h, int sunken);
void draw_string(int x, int y, const char* str, unsigned char color);
void trigger_bsod(const char* error_code);
void ata_write_sector(unsigned int lba, unsigned char* buffer);

void launch_calc_app(void);
void launch_settings_app(void);
void launch_filemgr_app(void);
void launch_taskman_app(void);
void launch_cver_app(void);
void launch_paint_app(void);
void launch_notepad_app(void);

#endif
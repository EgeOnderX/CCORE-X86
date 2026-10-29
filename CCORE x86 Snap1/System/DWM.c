#include "DWM.h"
MouseState mouse = {320, 240, 0, 0, 0, 0};
Window windows[8];
int window_count = 0;

#define backbuffer ((unsigned char*)0x200000)

unsigned char* vga_lfb = 0;
int start_menu_open = 0;
int run_dialog_open = 0;
char run_buffer[32] = "";
int run_buf_len = 0;
unsigned int system_ticks = 0;
int is_shutting_down = 0;

SystemSettings sys_settings = {0x3B, 1, 0, 64};

const unsigned char mouse_cursor[16][12] = {
    {1,0,0,0,0,0,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0,0,0,0}, {1,15,1,0,0,0,0,0,0,0,0,0},
    {1,15,15,1,0,0,0,0,0,0,0,0}, {1,15,15,15,1,0,0,0,0,0,0,0}, {1,15,15,15,15,1,0,0,0,0,0,0},
    {1,15,15,15,15,15,1,0,0,0,0,0}, {1,15,15,15,15,15,15,1,0,0,0,0}, {1,15,15,15,15,15,15,15,1,0,0,0},
    {1,15,15,15,15,1,1,1,1,1,0,0}, {1,15,15,1,15,1,0,0,0,0,0,0}, {1,15,1,0,1,15,1,0,0,0,0,0},
    {1,1,0,0,0,1,15,1,0,0,0,0}, {1,0,0,0,0,0,1,15,1,0,0,0}, {0,0,0,0,0,0,0,1,1,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0}
};

const unsigned char font8x8_basic[128][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['A'] = {0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C'] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, ['D'] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, ['F'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['G'] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}, ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, ['J'] = {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},
    ['K'] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, ['L'] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    ['M'] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, ['N'] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    ['O'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, ['P'] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['Q'] = {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00}, ['R'] = {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    ['S'] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, ['T'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['W'] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, ['X'] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, ['Z'] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
    ['a'] = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00}, ['b'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    ['c'] = {0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00}, ['d'] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00}, ['f'] = {0x1C, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x30, 0x00},
    ['g'] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C}, ['h'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i'] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00}, ['j'] = {0x06, 0x00, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['k'] = {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00}, ['l'] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x1C, 0x00},
    ['m'] = {0x00, 0x00, 0x66, 0x7F, 0x6B, 0x63, 0x63, 0x00}, ['n'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00}, ['p'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['q'] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06}, ['r'] = {0x00, 0x00, 0x3B, 0x6E, 0x60, 0x60, 0x60, 0x00},
    ['s'] = {0x00, 0x00, 0x3C, 0x60, 0x3C, 0x06, 0x3C, 0x00}, ['t'] = {0x10, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x0C, 0x00},
    ['u'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00}, ['v'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00}, ['x'] = {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00},
    ['y'] = {0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x3C, 0x00}, ['z'] = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00},
    ['0'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, ['1'] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['2'] = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00}, ['3'] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    ['4'] = {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00}, ['5'] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['6'] = {0x3C, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C, 0x00}, ['7'] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    ['8'] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, ['9'] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00},
    ['+'] = {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['*'] = {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, ['='] = {0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
    ['?'] = {0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00}, ['>'] = {0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00}
};

void put_pixel(int x, int y, unsigned char color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        backbuffer[y * SCREEN_W + x] = color;
    }
}

void flip_buffer(void) {
    while ((inb(0x3DA) & 0x08));
    while (!(inb(0x3DA) & 0x08));

    unsigned int* src = (unsigned int*)backbuffer;
    unsigned int* dst = (unsigned int*)vga_lfb;
    for (int i = 0; i < (SCREEN_W * SCREEN_H) / 4; i++) {
        dst[i] = src[i];
    }
}

void fill_rect(int x, int y, int w, int h, unsigned char color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

void draw_3d_box(int x, int y, int w, int h, int sunken) {
    unsigned char top_left = sunken ? 0x08 : 0x0F;
    unsigned char bot_right = sunken ? 0x0F : 0x08;

    fill_rect(x, y, w, h, 0x07);
    fill_rect(x, y, w, 1, top_left);
    fill_rect(x, y, 1, h, top_left);
    fill_rect(x, y + h - 1, w, 1, bot_right);
    fill_rect(x + w - 1, y, 1, h, bot_right);
}

void draw_string(int x, int y, const char* str, unsigned char color) {
    int cur_x = x;
    while (*str) {
        char c = *str;
        for (int row = 0; row < 8; row++) {
            unsigned char b = font8x8_basic[(int)c][row];
            for (int col = 0; col < 8; col++) {
                if (b & (0x80 >> col)) put_pixel(cur_x + col, y + row, color);
            }
        }
        cur_x += 8;
        str++;
    }
}

static unsigned char read_cmos(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd_to_bin(unsigned char val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

void get_rtc_time_str(char* out_str) {
    unsigned char status_b = read_cmos(0x0B);
    unsigned char h = read_cmos(0x04);
    unsigned char m = read_cmos(0x02);
    unsigned char s = read_cmos(0x00);

    int hour = (status_b & 0x04) ? h : bcd_to_bin(h);
    int min  = (status_b & 0x04) ? m : bcd_to_bin(m);
    int sec  = (status_b & 0x04) ? s : bcd_to_bin(s);

    out_str[0] = (hour / 10) + '0'; out_str[1] = (hour % 10) + '0'; out_str[2] = ':';
    out_str[3] = (min / 10) + '0'; out_str[4] = (min % 10) + '0'; out_str[5] = ':';
    out_str[6] = (sec / 10) + '0'; out_str[7] = (sec % 10) + '0'; out_str[8] = '\0';
}

void render_desktop_background(void) {
    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            if (sys_settings.bg_pattern_enabled && ((x + y) % 2 == 0)) {
                put_pixel(x, y, sys_settings.bg_color);
            } else {
                put_pixel(x, y, sys_settings.bg_color + (sys_settings.bg_pattern_enabled ? 1 : 0));
            }
        }
    }
}

void init_mouse_hardware(void) {
    outb(0x64, 0xA8);
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    inb(0x60);
}

void update_mouse(void) {
    static unsigned char prev_left = 0;
    unsigned char status = inb(0x64);

    if ((status & 0x01) && (status & 0x20)) {
        unsigned char b0 = inb(0x60);
        if ((b0 & 0x08) == 0) return;

        while (!(inb(0x64) & 0x01));
        char dx = (char)inb(0x60);

        while (!(inb(0x64) & 0x01));
        char dy = (char)inb(0x60);

        mouse.x += dx;
        mouse.y -= dy;

        if (mouse.x < 0) mouse.x = 0;
        if (mouse.x >= SCREEN_W - 12) mouse.x = SCREEN_W - 12;
        if (mouse.y < 0) mouse.y = 0;
        if (mouse.y >= SCREEN_H - 16) mouse.y = SCREEN_H - 16;

        mouse.left_btn = b0 & 0x01;
        mouse.right_btn = (b0 >> 1) & 0x01;

        if (mouse.left_btn && !prev_left) mouse.left_clicked = 1;
        if (!mouse.left_btn && prev_left) mouse.left_released = 1;

        prev_left = mouse.left_btn;
    } else if (status & 0x01) {
        inb(0x60);
    }
}

void sleep_ticks(int count) {
    for (volatile int i = 0; i < count * 50000; i++);
}

// Şık Kapanış Animasyonu
void start_shutdown_sequence(void) {
    is_shutting_down = 1;
    window_count = 0;
    start_menu_open = 0;
    run_dialog_open = 0;

    render_desktop_background();
    draw_string(180, 180, "CCORE is shutting down.", 0x0F);
    draw_string(180, 210, "> Stopping system services...", 0x0E);
    flip_buffer();
    sleep_ticks(400);

    render_desktop_background();
    draw_string(180, 180, "CCORE is shutting down.", 0x0F);
    draw_string(180, 210, "> Stopping system services... [OK]", 0x0A);
    draw_string(180, 230, "> Saving configs to CONFIG.CFG...", 0x0E);
    flip_buffer();
    sleep_ticks(400);

    render_desktop_background();
    draw_string(180, 180, "CCORE is shutting down.", 0x0F);
    draw_string(180, 210, "> Stopping system services... [OK]", 0x0A);
    draw_string(180, 230, "> Saving configs to CONFIG.CFG... [OK]", 0x0A);
    draw_string(180, 260, "System Halted. Safe to turn off.", 0x0F);
    flip_buffer();
    sleep_ticks(300);

    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    __asm__ __volatile__ ("cli; hlt");
}

void system_restart(void) {
    outb(0x64, 0xFE);
}

// Scancode Harf Dönüştürücü
static char scancode_to_ascii(unsigned char scancode) {
    switch (scancode) {
        case 0x1E: return 'a'; case 0x30: return 'b'; case 0x2E: return 'c'; case 0x20: return 'd';
        case 0x12: return 'e'; case 0x21: return 'f'; case 0x22: return 'g'; case 0x23: return 'h';
        case 0x17: return 'i'; case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
        case 0x32: return 'm'; case 0x31: return 'n'; case 0x18: return 'o'; case 0x19: return 'p';
        case 0x10: return 'q'; case 0x13: return 'r'; case 0x1F: return 's'; case 0x14: return 't';
        case 0x16: return 'u'; case 0x2F: return 'v'; case 0x11: return 'w'; case 0x2D: return 'x';
        case 0x15: return 'y'; case 0x2C: return 'z'; case 0x39: return ' '; default: return 0;
    }
}

void handle_keyboard(void) {
    static int ctrl_pressed = 0;
    if (inb(0x64) & 0x01) {
        unsigned char scancode = inb(0x60);
        if (scancode == 0x1D) ctrl_pressed = 1;
        if (scancode == 0x9D) ctrl_pressed = 0;

        // Ctrl + R
        if (ctrl_pressed && scancode == 0x13) {
            run_dialog_open = !run_dialog_open;
            run_buf_len = 0;
            run_buffer[0] = '\0';
            return;
        }

        if (run_dialog_open && !(scancode & 0x80)) {
            if (scancode == 0x1C) { // Enter
                if (run_buffer[0] == 'c' && run_buffer[1] == 'a') launch_calc_app();
                else if (run_buffer[0] == 'p' && run_buffer[1] == 'a') launch_paint_app();
                else if (run_buffer[0] == 'f' && run_buffer[1] == 'i') launch_filemgr_app();
                else if (run_buffer[0] == 't' && run_buffer[1] == 'a') launch_taskman_app();
                else if (run_buffer[0] == 's' && run_buffer[1] == 'e') launch_settings_app();
                else if (run_buffer[0] == 'a' && run_buffer[1] == 'b') launch_cver_app();
                run_dialog_open = 0;
            } else if (scancode == 0x0E && run_buf_len > 0) { // Backspace
                run_buffer[--run_buf_len] = '\0';
            } else {
                char ch = scancode_to_ascii(scancode);
                if (ch && run_buf_len < 20) {
                    run_buffer[run_buf_len++] = ch;
                    run_buffer[run_buf_len] = '\0';
                }
            }
        }
    }
}

void render_run_dialog(void) {
    if (!run_dialog_open) return;
    int rx = 180, ry = 180, rw = 280, rh = 90;
    draw_3d_box(rx, ry, rw, rh, 0);
    fill_rect(rx + 3, ry + 3, rw - 6, 16, 0x01);
    draw_string(rx + 6, ry + 7, "RUN PROGRAM (CTRL+R)", 0x0F);

    draw_string(rx + 10, ry + 28, "Type App (calc, paint, files, tasks):", 0x00);
    draw_3d_box(rx + 10, ry + 44, rw - 20, 20, 1);
    draw_string(rx + 15, ry + 50, run_buffer, 0x00);
}

void render_taskbar(void) {
    int tb_y = SCREEN_H - 24;
    draw_3d_box(0, tb_y, SCREEN_W, 24, 0);

    int start_pressed = start_menu_open || (mouse.left_btn && mouse.x >= 2 && mouse.x <= 60 && mouse.y >= tb_y + 2 && mouse.y <= tb_y + 22);
    draw_3d_box(2, tb_y + 2, 58, 20, start_pressed);
    draw_string(10 + (start_pressed?1:0), tb_y + 8 + (start_pressed?1:0), "START", 0x00);

    int btn_x = 65;
    for (int i = 0; i < window_count; i++) {
        if (!windows[i].is_visible) continue;
        int pressed = (mouse.left_btn && mouse.x >= btn_x && mouse.x <= btn_x + 80 && mouse.y >= tb_y + 2 && mouse.y <= tb_y + 22);
        draw_3d_box(btn_x, tb_y + 2, 80, 20, pressed);
        draw_string(btn_x + 6 + (pressed?1:0), tb_y + 8 + (pressed?1:0), windows[i].title, 0x00);
        btn_x += 85;
    }

    char time_buf[12];
    get_rtc_time_str(time_buf);
    draw_3d_box(SCREEN_W - 80, tb_y + 2, 76, 20, 1);
    draw_string(SCREEN_W - 72, tb_y + 8, time_buf, 0x00);

    if (start_menu_open) {
        int sm_y = tb_y - 200;
        draw_3d_box(2, sm_y, 150, 200, 0);

        fill_rect(4, sm_y + 4, 142, 22, 0x01);
        draw_string(10, sm_y + 11, "CCORE OS v1.0", 0x0F);

        draw_string(12, sm_y + 35, "RUN PROGRAM", 0x00);
        draw_string(12, sm_y + 55, "CALCULATOR", 0x00);
        draw_string(12, sm_y + 75, "PAINT", 0x00);
        draw_string(12, sm_y + 95, "FILES", 0x00);
        draw_string(12, sm_y + 115, "SETTINGS", 0x00);
        draw_string(12, sm_y + 135, "TASK MANAGER", 0x00);
        draw_string(12, sm_y + 155, "RESTART", 0x00);
        draw_string(12, sm_y + 175, "SHUTDOWN", 0x00);
    }
}

void handle_taskbar_clicks(int mx, int my) {
    int tb_y = SCREEN_H - 24;
    if (mx >= 2 && mx <= 60 && my >= tb_y + 2 && my <= tb_y + 22) {
        start_menu_open = !start_menu_open;
        return;
    }

    if (start_menu_open) {
        int sm_y = tb_y - 200;
        if (mx >= 2 && mx <= 152 && my >= sm_y && my <= tb_y) {
            if (my >= sm_y + 30 && my <= sm_y + 50) { run_dialog_open = 1; run_buf_len = 0; run_buffer[0] = '\0'; }
            if (my >= sm_y + 50 && my <= sm_y + 70) launch_calc_app();
            if (my >= sm_y + 70 && my <= sm_y + 90) launch_paint_app();
            if (my >= sm_y + 90 && my <= sm_y + 110) launch_filemgr_app();
            if (my >= sm_y + 110 && my <= sm_y + 130) launch_settings_app();
            if (my >= sm_y + 130 && my <= sm_y + 150) launch_taskman_app();
            if (my >= sm_y + 150 && my <= sm_y + 170) system_restart();
            if (my >= sm_y + 170 && my <= sm_y + 190) start_shutdown_sequence();
            start_menu_open = 0;
            return;
        } else {
            start_menu_open = 0;
        }
    }
}

void render_windows(void) {
    for (int i = 0; i < window_count; i++) {
        Window* w = &windows[i];
        if (!w->is_visible) continue;

        draw_3d_box(w->x, w->y, w->w, w->h, 0);
        fill_rect(w->x + 3, w->y + 3, w->w - 6, 16, 0x01);
        draw_string(w->x + 6, w->y + 7, w->title, 0x0F);

        int abtn_x = w->x + w->w - 32;
        int abtn_y = w->y + 4;
        draw_3d_box(abtn_x, abtn_y, 13, 13, w->about_btn_pressed ? 1 : 0);
        draw_string(abtn_x + 3 + (w->about_btn_pressed?1:0), abtn_y + 3 + (w->about_btn_pressed?1:0), "?", 0x00);

        int btn_x = w->x + w->w - 17;
        int btn_y = w->y + 4;
        draw_3d_box(btn_x, btn_y, 13, 13, w->x_btn_pressed ? 1 : 0);
        draw_string(btn_x + 3 + (w->x_btn_pressed?1:0), btn_y + 3 + (w->x_btn_pressed?1:0), "X", 0x00);

        if (w->render_content) w->render_content(w);
    }
}

void handle_window_events(void) {
    static Window* dragged_win = 0;

    if (mouse.left_clicked) {
        if (mouse.y >= SCREEN_H - 24) {
            handle_taskbar_clicks(mouse.x, mouse.y);
            return;
        }
        if (start_menu_open) {
            handle_taskbar_clicks(mouse.x, mouse.y);
            return;
        }

        for (int i = window_count - 1; i >= 0; i--) {
            Window* w = &windows[i];
            if (!w->is_visible) continue;

            if (mouse.x >= w->x && mouse.x <= w->x + w->w &&
                mouse.y >= w->y && mouse.y <= w->y + w->h) {

                if (mouse.y <= w->y + 20) {
                    int btn_x = w->x + w->w - 17;
                    int abtn_x = w->x + w->w - 32;

                    if (mouse.x >= btn_x && mouse.x <= btn_x + 13) {
                        w->x_btn_pressed = 1;
                    } else if (mouse.x >= abtn_x && mouse.x <= abtn_x + 13) {
                        w->about_btn_pressed = 1;
                    } else {
                        dragged_win = w;
                        w->drag_off_x = mouse.x - w->x;
                        w->drag_off_y = mouse.y - w->y;
                    }
                } 
                else if (w->click_event) {
                    w->click_event(w, mouse.x, mouse.y);
                }
                break;
            }
        }
    }

    if (mouse.left_btn && dragged_win) {
        dragged_win->x = mouse.x - dragged_win->drag_off_x;
        dragged_win->y = mouse.y - dragged_win->drag_off_y;
        if (dragged_win->y < 0) dragged_win->y = 0;
    }

    if (mouse.left_released) {
        dragged_win = 0;
        for (int i = 0; i < window_count; i++) {
            Window* w = &windows[i];
            if (w->x_btn_pressed) {
                w->is_visible = 0;
                w->x_btn_pressed = 0;
            }
            if (w->about_btn_pressed) {
                launch_cver_app();
                w->about_btn_pressed = 0;
            }
        }
    }
}

void draw_mouse_cursor(void) {
    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 12; c++) {
            unsigned char pixel = mouse_cursor[r][c];
            if (pixel == 1) put_pixel(mouse.x + c, mouse.y + r, 0x00);
            else if (pixel == 15) put_pixel(mouse.x + c, mouse.y + r, 0x0F);
        }
    }
}

void kmain(void) {
    vga_lfb = (unsigned char*)(*(unsigned int*)0x7E00);
    if ((unsigned int)vga_lfb == 0) vga_lfb = (unsigned char*)0xA0000;

    init_mouse_hardware();
    window_count = 0;

    while (1) {
        if (!is_shutting_down) {
            system_ticks++;
            update_mouse();
            handle_keyboard();
            handle_window_events();

            render_desktop_background();
            render_windows();
            render_run_dialog();
            render_taskbar();

            draw_mouse_cursor();

            flip_buffer();

            mouse.left_clicked = 0;
            mouse.left_released = 0;
        }
    }
}
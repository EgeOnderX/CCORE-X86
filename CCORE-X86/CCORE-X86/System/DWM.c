/* CCORE v0.2 - DWM.c : desktop window manager, drivers & core loop */
#include "DWM.h"

/* ===================== libc-lite ===================== */
void *memset(void *d, int c, u32 n) { void *r = d; __asm__ volatile("rep stosb" : "+D"(d), "+c"(n) : "a"(c) : "memory"); return r; }
void *memcpy(void *d, const void *s, u32 n) { void *r = d; __asm__ volatile("rep movsb" : "+D"(d), "+S"(s), "+c"(n) : : "memory"); return r; }
void *memmove(void *d, const void *s, u32 n) {
    if (d <= s || (u8 *)d >= (u8 *)s + n) return memcpy(d, s, n);
    u8 *dd = (u8 *)d + n; const u8 *ss = (const u8 *)s + n;
    while (n--) *--dd = *--ss;
    return d;
}
u32 strlen(const char *s) { u32 n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (u8)*a - (u8)*b; }
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)) {} return r; }
void str_ncpy(char *d, const char *s, int n) { int i = 0; while (i < n - 1 && s[i]) { d[i] = s[i]; i++; } d[i] = 0; }
static int up(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }
int str_eq_nocase(const char *a, const char *b) { while (*a && up(*a) == up(*b)) { a++; b++; } return *a == *b; }
char *fmt_u(char *d, u32 v) { char t[11]; int n = 0; do { t[n++] = '0' + v % 10; v /= 10; } while (v); while (n) *d++ = t[--n]; *d = 0; return d; }
char *fmt_hex(char *d, u32 v, int digits) { for (int i = digits - 1; i >= 0; i--) *d++ = "0123456789ABCDEF"[(v >> (i * 4)) & 15]; *d = 0; return d; }
char *fmt_2d(char *d, u32 v) { *d++ = '0' + (v / 10) % 10; *d++ = '0' + v % 10; *d = 0; return d; }
char *fmt_str(char *d, const char *s) { while (*s) *d++ = *s++; *d = 0; return d; }
void fmt_hms(char *d, u32 s) { d = fmt_2d(d, s / 3600); *d++ = ':'; d = fmt_2d(d, (s / 60) % 60); *d++ = ':'; fmt_2d(d, s % 60); }

/* ===================== 8x8 font (bit0 = left pixel) ===================== */
static const u8 font8x8[95][8] = {
{0,0,0,0,0,0,0,0},{0x18,0x3C,0x3C,0x18,0x18,0,0x18,0},{0x36,0x36,0,0,0,0,0,0},{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0},
{0x0C,0x3E,3,0x1E,0x30,0x1F,0x0C,0},{0,0x63,0x33,0x18,0x0C,0x66,0x63,0},{0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0},{6,6,3,0,0,0,0,0},
{0x18,0x0C,6,6,6,0x0C,0x18,0},{6,0x0C,0x18,0x18,0x18,0x0C,6,0},{0,0x66,0x3C,0xFF,0x3C,0x66,0,0},{0,0x0C,0x0C,0x3F,0x0C,0x0C,0,0},
{0,0,0,0,0,0x0C,0x0C,6},{0,0,0,0x3F,0,0,0,0},{0,0,0,0,0,0x0C,0x0C,0},{0x60,0x30,0x18,0x0C,6,3,1,0},
{0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0},{0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0},{0x1E,0x33,0x30,0x1C,6,0x33,0x3F,0},{0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0},
{0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0},{0x3F,3,0x1F,0x30,0x30,0x33,0x1E,0},{0x1C,6,3,0x1F,0x33,0x33,0x1E,0},{0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0},
{0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0},{0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0},{0,0x0C,0x0C,0,0,0x0C,0x0C,0},{0,0x0C,0x0C,0,0,0x0C,0x0C,6},
{0x18,0x0C,6,3,6,0x0C,0x18,0},{0,0,0x3F,0,0,0x3F,0,0},{6,0x0C,0x18,0x30,0x18,0x0C,6,0},{0x1E,0x33,0x30,0x18,0x0C,0,0x0C,0},
{0x3E,0x63,0x7B,0x7B,0x7B,3,0x1E,0},{0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0},{0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0},{0x3C,0x66,3,3,3,0x66,0x3C,0},
{0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0},{0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0},{0x7F,0x46,0x16,0x1E,0x16,6,0x0F,0},{0x3C,0x66,3,3,0x73,0x66,0x7C,0},
{0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0},{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},{0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0},{0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0},
{0x0F,6,6,6,0x46,0x66,0x7F,0},{0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0},{0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0},{0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0},
{0x3F,0x66,0x66,0x3E,6,6,0x0F,0},{0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0},{0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0},{0x1E,0x33,7,0x0E,0x38,0x33,0x1E,0},
{0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0},{0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0},{0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0},{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0},
{0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0},{0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0},{0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0},{0x1E,6,6,6,6,6,0x1E,0},
{3,6,0x0C,0x18,0x30,0x60,0x40,0},{0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0},{8,0x1C,0x36,0x63,0,0,0,0},{0,0,0,0,0,0,0,0xFF},
{0x0C,0x0C,0x18,0,0,0,0,0},{0,0,0x1E,0x30,0x3E,0x33,0x6E,0},{7,6,6,0x3E,0x66,0x66,0x3B,0},{0,0,0x1E,0x33,3,0x33,0x1E,0},
{0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0},{0,0,0x1E,0x33,0x3F,3,0x1E,0},{0x1C,0x36,6,0x0F,6,6,0x0F,0},{0,0,0x6E,0x33,0x33,0x3E,0x30,0x1F},
{7,6,0x36,0x6E,0x66,0x66,0x67,0},{0x0C,0,0x0E,0x0C,0x0C,0x0C,0x1E,0},{0x30,0,0x30,0x30,0x30,0x33,0x33,0x1E},{7,6,0x66,0x36,0x1E,0x36,0x67,0},
{0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},{0,0,0x33,0x7F,0x7F,0x6B,0x63,0},{0,0,0x1F,0x33,0x33,0x33,0x33,0},{0,0,0x1E,0x33,0x33,0x33,0x1E,0},
{0,0,0x3B,0x66,0x66,0x3E,6,0x0F},{0,0,0x6E,0x33,0x33,0x3E,0x30,0x78},{0,0,0x3B,0x6E,0x66,6,0x0F,0},{0,0,0x3E,3,0x1E,0x30,0x1F,0},
{8,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0},{0,0,0x33,0x33,0x33,0x33,0x6E,0},{0,0,0x33,0x33,0x33,0x1E,0x0C,0},{0,0,0x63,0x6B,0x7F,0x7F,0x36,0},
{0,0,0x63,0x36,0x1C,0x36,0x63,0},{0,0,0x33,0x33,0x33,0x3E,0x30,0x1F},{0,0,0x3F,0x19,0x0C,0x26,0x3F,0},{0x38,0x0C,0x0C,7,0x0C,0x0C,0x38,0},
{0x18,0x18,0x18,0,0x18,0x18,0x18,0},{7,0x0C,0x0C,0x38,0x0C,0x0C,7,0},{0x6E,0x3B,0,0,0,0,0,0}
};

/* ===================== graphics ===================== */
#define backbuf ((u8 *)0x200000)             /* up to 720x480 backbuffer, above 1 MB */
#define wallbuf ((u8 *)0x400000)
int scr_w = 320, scr_h = 200, cur_res, vbe_ok;
const int res_w[3] = { 320, 640, 720 }, res_h[3] = { 200, 480, 480 };
const char *const res_names[3] = { "320 x 200 VGA", "640 x 480", "720 x 480" };
static u8 *fb = (u8 *)0xA0000;
static u32 lfb_addr;
static int gx0, gy0, cl_x0 = 0, cl_y0 = 0, cl_x1 = 320, cl_y1 = 200;
int press_x = -1, press_y = -1, pressing;
u32 release_tick;
/* a button looks pressed (sunken) while held, and briefly after release */
int btn_hot(int x, int y, int w, int h) {
    if (!pressing && sys_ticks - release_tick >= 8) return 0;
    if (!ui_in(press_x, press_y, x, y, w, h)) return 0;
    return !pressing || ui_in(mouse_x, mouse_y, x, y, w, h);
}

void gfx_set_origin(int x, int y) { gx0 = x; gy0 = y; }
void gfx_set_clip(int x0, int y0, int x1, int y1) {
    cl_x0 = x0 < 0 ? 0 : x0; cl_y0 = y0 < 0 ? 0 : y0;
    cl_x1 = x1 > SCR_W ? SCR_W : x1; cl_y1 = y1 > SCR_H ? SCR_H : y1;
}
void gfx_pixel(int x, int y, u8 c) {
    x += gx0; y += gy0;
    if (x < cl_x0 || x >= cl_x1 || y < cl_y0 || y >= cl_y1) return;
    backbuf[y * SCR_W + x] = c;
}
void gfx_rect(int x, int y, int w, int h, u8 c) {
    x += gx0; y += gy0;
    int x1 = x + w, y1 = y + h;
    if (x < cl_x0) x = cl_x0;
    if (y < cl_y0) y = cl_y0;
    if (x1 > cl_x1) x1 = cl_x1;
    if (y1 > cl_y1) y1 = cl_y1;
    if (x1 <= x || y1 <= y) return;
    for (int j = y; j < y1; j++) memset(&backbuf[j * SCR_W + x], c, x1 - x);
}
void gfx_hline(int x, int y, int w, u8 c) { gfx_rect(x, y, w, 1, c); }
void gfx_vline(int x, int y, int h, u8 c) { gfx_rect(x, y, 1, h, c); }
void gfx_frame(int x, int y, int w, int h, u8 c) {
    gfx_hline(x, y, w, c); gfx_hline(x, y + h - 1, w, c);
    gfx_vline(x, y, h, c); gfx_vline(x + w - 1, y, h, c);
}
void gfx_bevel(int x, int y, int w, int h, int raised) {
    u8 tl = raised ? C_WHITE : C_DGRAY, br = raised ? C_DGRAY : C_WHITE;
    gfx_rect(x, y, w, h, C_LGRAY);
    gfx_hline(x, y, w, tl); gfx_vline(x, y, h, tl);
    gfx_hline(x, y + h - 1, w, br); gfx_vline(x + w - 1, y, h, br);
}
void gfx_sunken(int x, int y, int w, int h, u8 bg) {
    gfx_rect(x, y, w, h, bg);
    gfx_hline(x, y, w, C_DGRAY); gfx_vline(x, y, h, C_DGRAY);
    gfx_hline(x, y + h - 1, w, C_WHITE); gfx_vline(x + w - 1, y, h, C_WHITE);
}
void gfx_char(int x, int y, int ch, u8 c) {
    if (ch < 32 || ch > 126) return;
    const u8 *g = font8x8[ch - 32];
    for (int r = 0; r < 8; r++)
        for (int k = 0; k < 7; k++)
            if (g[r] & (1 << k)) gfx_pixel(x + k, y + r, c);
}
void gfx_text(int x, int y, const char *s, u8 c) { for (; *s; s++, x += 8) gfx_char(x, y, *s, c); }
void gfx_text_n(int x, int y, const char *s, u8 c, int n) { for (; *s && n > 0; s++, x += 8, n--) gfx_char(x, y, *s, c); }
void gfx_text_big(int x, int y, const char *s, u8 c, int sc) {
    for (; *s; s++, x += 8 * sc) {
        if (*s < 32 || *s > 126) continue;
        const u8 *g = font8x8[*s - 32];
        for (int r = 0; r < 8; r++)
            for (int k = 0; k < 7; k++)
                if (g[r] & (1 << k)) gfx_rect(x + k * sc, y + r * sc, sc, sc, c);
    }
}
void gfx_button(int x, int y, int w, int h, const char *label, int pressed) {
    pressed |= btn_hot(x + gx0, y + gy0, w, h);
    gfx_frame(x, y, w, h, C_BLACK);
    gfx_bevel(x + 1, y + 1, w - 2, h - 2, !pressed);
    int tw = strlen(label) * 8 - 1, o = pressed ? 1 : 0;
    gfx_text(x + (w - tw) / 2 + o, y + (h - 8) / 2 + o, label, C_BLACK);
}
void gfx_blit(int x, int y, int w, int h, const u8 *src) {
    for (int j = 0; j < h; j++) {
        int dy = y + j + gy0, dx = x + gx0, sx = 0, n = w;
        if (dy < cl_y0 || dy >= cl_y1) continue;
        if (dx < cl_x0) { sx = cl_x0 - dx; n -= sx; dx = cl_x0; }
        if (dx + n > cl_x1) n = cl_x1 - dx;
        if (n > 0) memcpy(&backbuf[dy * SCR_W + dx], src + j * w + sx, n);
    }
}
void gfx_vscroll(int x, int y, int w, int h, int pos, int total, int visible) {
    gfx_sunken(x, y, w, h, C_LGRAY);
    if (total <= visible || total <= 0) return;
    int th = (h - 2) * visible / total; if (th < 6) th = 6;
    int ty = y + 1 + (h - 2 - th) * pos / (total - visible);
    gfx_bevel(x + 1, ty, w - 2, th, 1);
}
void gfx_flip(void) {
    void *d = fb; const void *s = backbuf; u32 n = SCR_W * SCR_H / 4;
    __asm__ volatile("rep movsl" : "+D"(d), "+S"(s), "+c"(n) : : "memory");
}

/* Bochs/QEMU VBE (dispi) linear framebuffer modes; mode 13h stays the default */
static void dispi(u16 i, u16 v) { outw(0x1CE, i); outw(0x1CF, v); }
u32 gfx_fb_addr(void) { return (u32)fb; }
void vbe_init(void) {
    vbe_ok = 0;
    for (u32 dev = 0; dev < 32; dev++) {
        u32 a = 0x80000000u | (dev << 11);
        outl(0xCF8, a); if (inl(0xCFC) == 0xFFFFFFFFu) continue;
        outl(0xCF8, a | 8); if ((inl(0xCFC) >> 24) != 3) continue;          /* display controller */
        outl(0xCF8, a | 0x10); u32 bar = inl(0xCFC);
        if ((bar & 1) || !(bar & ~0xFu)) continue;
        outw(0x1CE, 0); u16 id = inw(0x1CF);
        if (id < 0xB0C0 || id > 0xB0CF) continue;
        outl(0xCF8, a | 4); u32 cmd = inl(0xCFC); outl(0xCF8, a | 4); outl(0xCFC, cmd | 7);
        lfb_addr = bar & ~0xFu; vbe_ok = 1; return;
    }
}
static int gfx_set_res(int r) {
    if (r < 0 || r > 2) return -1;
    if (r == cur_res) return 0;
    if (r > 0) {
        if (!vbe_ok) return -1;
        dispi(4, 0); dispi(1, res_w[r]); dispi(2, res_h[r]); dispi(3, 8); dispi(6, res_w[r]); dispi(4, 0x41);
        fb = (u8 *)lfb_addr;
    } else {
        if (vbe_ok) dispi(4, 0);
        fb = (u8 *)0xA0000;
    }
    scr_w = res_w[r]; scr_h = res_h[r]; cur_res = r;
    cl_x0 = 0; cl_y0 = 0; cl_x1 = scr_w; cl_y1 = scr_h;
    mouse_x = scr_w / 2; mouse_y = scr_h / 2;
    return 0;
}

/* ===================== settings / themes / wallpaper ===================== */
Settings cfg = { .wall_color = C_CYAN, .wall_pattern = 0, .theme = 0, .trails = 0, .trail_len = 6, .mouse_speed = 4, .res = 0 };
const char *const theme_names[NUM_THEMES] = { "Windows Default", "Ocean", "Plum", "Forest", "Hot Dog Stand" };
const u8 theme_active[NUM_THEMES] = { C_BLUE, C_CYAN, C_MAGENTA, C_GREEN, C_RED };
const u8 theme_text[NUM_THEMES] = { C_WHITE, C_BLACK, C_WHITE, C_WHITE, C_YELLOW };
const char *const pattern_names[NUM_PATTERNS] = { "Solid", "Checker", "Diagonal", "Dots", "Grid" };
char cur_user[16] = "", acct_user[16] = "", acct_pass[16] = "", admin_pass[16] = "admin";
int cur_admin;
int has_user;

void wallpaper_rebuild(void) {
    u8 a = cfg.wall_color, b = (a == C_WHITE) ? C_LGRAY : (a == C_BLACK ? C_DGRAY : (a < 8 ? a + 8 : a - 8));
    for (int y = 0; y < SCR_H; y++)
        for (int x = 0; x < SCR_W; x++) {
            u8 c = a;
            switch (cfg.wall_pattern) {
            case 1: if ((x + y) & 1) c = b; break;
            case 2: if (((x + y) & 7) < 2) c = b; break;
            case 3: if ((x & 7) < 2 && (y & 7) < 2) c = b; break;
            case 4: if ((x & 15) == 0 || (y & 15) == 0) c = b; break;
            }
            wallbuf[y * SCR_W + x] = c;
        }
}

/* ===================== ATA PIO driver (28-bit LBA, primary master) ===================== */
int ata_ok, fs_ok;
u32 ata_sectors;
char ata_model[41];
static int ata_wait(int need_drq) {
    for (int i = 0; i < 400000; i++) {
        u8 s = inb(0x1F7);
        if (s & 0x01 || s & 0x20) return -1;
        if (!(s & 0x80) && (!need_drq || (s & 0x08))) return 0;
    }
    return -1;
}
static void ata_select(u32 lba) { outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); for (int i = 0; i < 4; i++) inb(0x3F6); }
static void ata_init(void) {
    ata_ok = 0;
    outb(0x1F6, 0xA0);
    for (int i = 0; i < 4; i++) inb(0x3F6);
    if (inb(0x1F7) == 0xFF) return;
    outb(0x1F2, 0); outb(0x1F3, 0); outb(0x1F4, 0); outb(0x1F5, 0); outb(0x1F7, 0xEC);
    if (inb(0x1F7) == 0) return;
    for (int i = 0; i < 400000 && (inb(0x1F7) & 0x80); i++) {}
    if (inb(0x1F4) || inb(0x1F5)) return;
    if (ata_wait(1)) return;
    u16 id[256];
    for (int i = 0; i < 256; i++) id[i] = inw(0x1F0);
    for (int i = 0; i < 20; i++) { ata_model[i * 2] = id[27 + i] >> 8; ata_model[i * 2 + 1] = id[27 + i] & 0xFF; }
    ata_model[40] = 0;
    for (int i = 39; i >= 0 && ata_model[i] == ' '; i--) ata_model[i] = 0;
    ata_sectors = id[60] | ((u32)id[61] << 16);
    ata_ok = 1;
}
int ata_read(u32 lba, u32 count, void *buf) {
    if (!ata_ok) return -1;
    u16 *p = (u16 *)buf;
    for (u32 s = 0; s < count; s++, lba++) {
        if (ata_wait(0)) return -1;
        ata_select(lba);
        outb(0x1F2, 1); outb(0x1F3, lba & 0xFF); outb(0x1F4, (lba >> 8) & 0xFF); outb(0x1F5, (lba >> 16) & 0xFF);
        outb(0x1F7, 0x20);
        if (ata_wait(1)) return -1;
        for (int i = 0; i < 256; i++) *p++ = inw(0x1F0);
    }
    return 0;
}
int ata_write(u32 lba, u32 count, const void *buf) {
    if (!ata_ok) return -1;
    const u16 *p = (const u16 *)buf;
    for (u32 s = 0; s < count; s++, lba++) {
        if (ata_wait(0)) return -1;
        ata_select(lba);
        outb(0x1F2, 1); outb(0x1F3, lba & 0xFF); outb(0x1F4, (lba >> 8) & 0xFF); outb(0x1F5, (lba >> 16) & 0xFF);
        outb(0x1F7, 0x30);
        if (ata_wait(1)) return -1;
        for (int i = 0; i < 256; i++) outw(0x1F0, *p++);
        outb(0x1F7, 0xE7);                      /* flush cache */
        if (ata_wait(0)) return -1;
    }
    return 0;
}

/* ===================== CFS: tiny file system on top of ATA ===================== */
#define FS_MAGIC 0x53464343u   /* "CCFS" */
#define FS_SUPER_LBA 300
#define FS_DIR_LBA 301
#define FS_DIR_SECTS 4
#define FS_DATA_LBA 400
#define NICONS 8
static int icon_x[NICONS], icon_y[NICONS];
static char icon_name[NICONS][9];
static const char *const icon_def_name[NICONS] = { "Calc", "Notepad", "Paint", "Files", "Control", "TaskMgr", "About", "Cmd" };
void icons_arrange(void) {
    for (int i = 0; i < NICONS; i++) {
        icon_x[i] = i < 4 ? 6 : 62; icon_y[i] = 6 + (i % 4) * 36;
        str_ncpy(icon_name[i], icon_def_name[i], 9);
    }
}
static union { struct { u32 magic, version; char user[16], pass[16]; Settings cfg; u16 ix[NICONS], iy[NICONS]; char iname[NICONS][9]; u8 icons_set, has_user; char apass[16]; } d; u8 raw[512]; } sb;
DirEntry fs_dir[MAX_FILES];

static int fs_dir_save(void) { return ata_write(FS_DIR_LBA, FS_DIR_SECTS, fs_dir); }
int fs_rescan(void) { return ata_read(FS_DIR_LBA, FS_DIR_SECTS, fs_dir); }
void settings_save(void) {
    if (!fs_ok) return;
    sb.d.cfg = cfg; str_ncpy(sb.d.user, acct_user, 16); str_ncpy(sb.d.pass, acct_pass, 16);
    sb.d.has_user = has_user; sb.d.icons_set = 1; str_ncpy(sb.d.apass, admin_pass, 16);
    for (int i = 0; i < NICONS; i++) { sb.d.ix[i] = icon_x[i]; sb.d.iy[i] = icon_y[i]; str_ncpy(sb.d.iname[i], icon_name[i], 9); }
    ata_write(FS_SUPER_LBA, 1, sb.raw);
}
int fs_find(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) if (fs_dir[i].used && str_eq_nocase(fs_dir[i].name, name)) return i;
    return -1;
}
int fs_create(const char *name) {
    if (!fs_ok) return -3;
    int n = strlen(name);
    if (n == 0 || n > NAME_MAX_LEN) return -4;
    if (fs_find(name) >= 0) return -2;
    for (int i = 0; i < MAX_FILES; i++)
        if (!fs_dir[i].used) {
            memset(&fs_dir[i], 0, sizeof(DirEntry));
            str_ncpy(fs_dir[i].name, name, 20);
            fs_dir[i].lba = FS_DATA_LBA + i * FILE_SECT; fs_dir[i].used = 1;
            return fs_dir_save() ? -3 : i;
        }
    return -1;
}
int fs_is_sys(int idx) { return idx >= 0 && idx < MAX_FILES && fs_dir[idx].used && (fs_dir[idx].pad[0] & DE_SYS); }
int fs_delete(int idx) {
    if (!fs_ok || idx < 0 || idx >= MAX_FILES || !fs_dir[idx].used) return -1;
    if (fs_is_sys(idx) && !cur_admin) return -5;
    fs_dir[idx].used = 0; fs_dir[idx].size = 0;
    return fs_dir_save();
}
int fs_read(int idx, void *buf, u32 max) {
    if (!fs_ok || idx < 0 || !fs_dir[idx].used) return -1;
    u32 size = fs_dir[idx].size < max ? fs_dir[idx].size : max, done = 0;
    static u8 sec[512];
    for (u32 s = 0; done < size; s++) {
        if (ata_read(fs_dir[idx].lba + s, 1, sec)) return -1;
        u32 n = size - done > 512 ? 512 : size - done;
        memcpy((u8 *)buf + done, sec, n); done += n;
    }
    return (int)size;
}
static int fs_write_raw(int idx, const void *buf, u32 size) {
    if (!fs_ok || idx < 0 || !fs_dir[idx].used || size > FILE_MAXB) return -1;
    static u8 sec[512];
    for (u32 done = 0, s = 0; done < size; s++) {
        u32 n = size - done > 512 ? 512 : size - done;
        memset(sec, 0, 512); memcpy(sec, (const u8 *)buf + done, n);
        if (ata_write(fs_dir[idx].lba + s, 1, sec)) return -1;
        done += n;
    }
    fs_dir[idx].size = size;
    return fs_dir_save();
}
int fs_write(int idx, const void *buf, u32 size) {
    if (fs_is_sys(idx) && !cur_admin) return -5;
    return fs_write_raw(idx, buf, size);
}
int fs_rename(int idx, const char *nn) {
    if (!fs_ok || idx < 0 || idx >= MAX_FILES || !fs_dir[idx].used) return -1;
    if (fs_is_sys(idx) && !cur_admin) return -5;
    int n = strlen(nn), o = fs_find(nn);
    if (n == 0 || n > NAME_MAX_LEN) return -4;
    if (o >= 0 && o != idx) return -2;
    str_ncpy(fs_dir[idx].name, nn, 20);
    return fs_dir_save() ? -3 : 0;
}
int fs_set_sys(int idx, int on) {
    if (!fs_ok || idx < 0 || idx >= MAX_FILES || !fs_dir[idx].used) return -1;
    if (!cur_admin) return -5;
    if (on) fs_dir[idx].pad[0] |= DE_SYS; else fs_dir[idx].pad[0] &= ~DE_SYS;
    return fs_dir_save() ? -3 : 0;
}
int fs_copy(int src, const char *dst) {
    static u8 cbuf[FILE_MAXB];
    if (fs_is_sys(src) && !cur_admin) return -5;
    int n = fs_read(src, cbuf, FILE_MAXB);
    if (n < 0) return -1;
    int d = fs_create(dst);
    if (d < 0) return d;
    if (fs_write_raw(d, cbuf, n)) return -3;
    return d;
}
static const char readme_txt[] =
    "Welcome to CCORE 0.2!\n\nThis file lives on the ATA disk image.\n"
    "Edit it in Notepad and press Save - it persists across reboots.\n";
static void put_file(const char *name, const char *txt, int sys) {
    int i = fs_create(name);
    if (i < 0) return;
    fs_write_raw(i, txt, strlen(txt));
    if (sys) { fs_dir[i].pad[0] |= DE_SYS; fs_dir_save(); }
}
int fs_format(const char *admin_pw, const char *user, const char *pass) {
    if (!ata_ok) return -1;
    str_ncpy(admin_pass, admin_pw && admin_pw[0] ? admin_pw : "admin", 16);
    memset(sb.raw, 0, 512); memset(fs_dir, 0, sizeof fs_dir);
    sb.d.magic = FS_MAGIC; sb.d.version = 3;
    str_ncpy(acct_user, user, 16); str_ncpy(acct_pass, pass, 16); has_user = acct_user[0] != 0;
    fs_ok = 1; settings_save();
    if (fs_dir_save()) { fs_ok = 0; return -1; }
    put_file("README.TXT", readme_txt, 0);
    put_file("CONFIG.SYS", "DEVICE=KEYBOARD.DRV\nDEVICE=MOUSE.DRV\nDEVICE=ATA.DRV\nFILES=64\n", 1);
    put_file("AUTOEXEC.BAT", "@ECHO OFF\nREM CCORE startup file\nECHO Type HELP for a list of commands.\n", 1);
    put_file("DWM.SYS", "[DWM]\nSystem file of the desktop window manager.\nOnly an administrator may change it.\n", 1);
    return 0;
}
static void fs_mount(void) {
    fs_ok = 0;
    if (!ata_ok) return;
    if (ata_read(FS_SUPER_LBA, 1, sb.raw)) return;
    if (sb.d.magic != FS_MAGIC) { fs_format("admin", "", ""); return; }      /* first boot: format, no account yet */
    fs_ok = 1;
    cfg = sb.d.cfg;
    if (cfg.theme >= NUM_THEMES) cfg.theme = 0;
    if (cfg.wall_pattern >= NUM_PATTERNS) cfg.wall_pattern = 0;
    if (cfg.wall_color > 15) cfg.wall_color = C_CYAN;
    if (cfg.trail_len < 1 || cfg.trail_len > 15) cfg.trail_len = 6;
    if (cfg.mouse_speed < 1 || cfg.mouse_speed > 8) cfg.mouse_speed = 4;
    str_ncpy(acct_user, sb.d.user, 16); str_ncpy(acct_pass, sb.d.pass, 16); str_ncpy(admin_pass, sb.d.apass, 16);
    if (!admin_pass[0]) strcpy(admin_pass, "admin");
    has_user = sb.d.has_user && acct_user[0];
    if (sb.d.icons_set)
        for (int i = 0; i < NICONS; i++) {
            if (sb.d.ix[i] <= 720 - 56 && sb.d.iy[i] <= 480 - 45) { icon_x[i] = sb.d.ix[i]; icon_y[i] = sb.d.iy[i]; }
            sb.d.iname[i][8] = 0;
            if (sb.d.iname[i][0]) str_ncpy(icon_name[i], sb.d.iname[i], 9);
        }
    fs_rescan();
}
void settings_apply(void) { wallpaper_rebuild(); }

/* ===================== input: keyboard (IRQ1) & PS/2 mouse (IRQ12) ===================== */
volatile u32 sys_ticks;
u32 ram_kb;
char cpu_name[49];
int cpu_ok;
int mouse_found;

#define KQ 64
static volatile int kq[KQ];
static volatile u32 kq_h, kq_t;
static int k_shift, k_caps, k_ext;
static void kq_push(int k) { u32 n = (kq_h + 1) % KQ; if (n != kq_t) { kq[kq_h] = k; kq_h = n; } }
int kbd_get(void) { if (kq_t == kq_h) return -1; int k = kq[kq_t]; kq_t = (kq_t + 1) % KQ; return k; }
static const char sc_lo[58] = { 0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,9,
    'q','w','e','r','t','y','u','i','o','p','[',']',10,0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ' };
static int shift_char(int c) {
    switch (c) {
    case '1': return '!'; case '2': return '@'; case '3': return '#'; case '4': return '$'; case '5': return '%';
    case '6': return '^'; case '7': return '&'; case '8': return '*'; case '9': return '('; case '0': return ')';
    case '-': return '_'; case '=': return '+'; case '[': return '{'; case ']': return '}'; case ';': return ':';
    case '\'': return '"'; case '`': return '~'; case '\\': return '|'; case ',': return '<'; case '.': return '>';
    case '/': return '?';
    }
    return c;
}
static void irq_keyboard(void) {
    u8 sc = inb(0x60);
    if (sc == 0xE0) { k_ext = 1; return; }
    int ext = k_ext, rel = sc & 0x80; k_ext = 0; sc &= 0x7F;
    if (sc == 0x2A || sc == 0x36) { if (!ext) k_shift = !rel; return; }
    if (sc == 0x1D || sc == 0x38) return;
    if (!ext && sc == 0x3A) { if (!rel) k_caps ^= 1; return; }
    if (rel) return;
    int k = 0;
    if (ext) {
        switch (sc) {
        case 0x48: k = KEY_UP; break; case 0x50: k = KEY_DOWN; break; case 0x4B: k = KEY_LEFT; break;
        case 0x4D: k = KEY_RIGHT; break; case 0x47: k = KEY_HOME; break; case 0x4F: k = KEY_END; break;
        case 0x53: k = KEY_DEL; break; case 0x49: k = KEY_PGUP; break; case 0x51: k = KEY_PGDN; break;
        case 0x1C: k = KEY_ENTER; break; case 0x35: k = '/'; break;
        }
    } else if (sc >= 0x47 && sc <= 0x53) {
        k = "789-456+1230."[sc - 0x47];
    } else if (sc < 58) {
        k = sc_lo[sc];
        if (k >= 'a' && k <= 'z') { if (k_shift ^ k_caps) k -= 32; }
        else if (k_shift) k = shift_char(k);
    }
    if (k) kq_push(k);
}

int mouse_x = 160, mouse_y = 100;
volatile int mouse_btn;
typedef struct { int x, y, btn; } MQ;
static MQ mq[64];
static u32 mq_h, mq_t;
static u8 mpkt[3];
static int mcnt;
static void irq_mouse(void) {
    u8 d = inb(0x60);
    if (mcnt == 0 && !(d & 8)) return;
    mpkt[mcnt++] = d;
    if (mcnt < 3) return;
    mcnt = 0;
    if (mpkt[0] & 0xC0) return;
    int dx = mpkt[1] - ((mpkt[0] & 0x10) ? 256 : 0), dy = mpkt[2] - ((mpkt[0] & 0x20) ? 256 : 0);
    dy = -dy;
    int nx = dx * cfg.mouse_speed / 4, ny = dy * cfg.mouse_speed / 4;
    if (!nx && dx) nx = dx > 0 ? 1 : -1;
    if (!ny && dy) ny = dy > 0 ? 1 : -1;
    mouse_btn = mpkt[0] & 3;
    mouse_x += nx; mouse_y += ny;
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= SCR_W) mouse_x = SCR_W - 1;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y >= SCR_H) mouse_y = SCR_H - 1;
    u32 n = (mq_h + 1) % 64;
    if (n != mq_t) { mq[mq_h].x = mouse_x; mq[mq_h].y = mouse_y; mq[mq_h].btn = mpkt[0] & 3; mq_h = n; }
}
static int mq_pending, prev_btn, last_x = -1, last_y = -1;
static MQ cur_ev;
int mouse_next(MouseEvt *e) {
    for (;;) {
        if (!mq_pending) {
            if (mq_h == mq_t) return 0;
            cur_ev = mq[mq_t]; mq_t = (mq_t + 1) % 64; mq_pending = 1;
        }
        int diff = (cur_ev.btn ^ prev_btn) & 3;
        if (diff) {
            int bit = (diff & 1) ? 1 : 2;
            e->type = (cur_ev.btn & bit) ? MOUSE_DOWN : MOUSE_UP;
            e->btn = bit; e->x = cur_ev.x; e->y = cur_ev.y;
            prev_btn ^= bit;
            if (bit == 1) {
                if (e->type == MOUSE_DOWN) { pressing = 1; press_x = e->x; press_y = e->y; }
                else { pressing = 0; release_tick = sys_ticks; }
            }
            return 1;
        }
        mq_pending = 0;
        if (cur_ev.x != last_x || cur_ev.y != last_y) {
            last_x = cur_ev.x; last_y = cur_ev.y;
            e->type = MOUSE_MOVE; e->btn = prev_btn; e->x = cur_ev.x; e->y = cur_ev.y;
            return 1;
        }
    }
}
static void kbc_wait_in(void) { for (int t = 0; t < 100000 && (inb(0x64) & 2); t++) {} }
static int kbc_wait_out(void) { for (int t = 0; t < 100000; t++) if (inb(0x64) & 1) return 1; return 0; }
static int mouse_cmd(u8 v) {
    kbc_wait_in(); outb(0x64, 0xD4); kbc_wait_in(); outb(0x60, v);
    return kbc_wait_out() && inb(0x60) == 0xFA;
}
static void mouse_init(void) {
    while (inb(0x64) & 1) inb(0x60);
    kbc_wait_in(); outb(0x64, 0xA8);
    kbc_wait_in(); outb(0x64, 0x20); kbc_wait_out();
    u8 st = inb(0x60); st |= 2; st &= ~0x20;
    kbc_wait_in(); outb(0x64, 0x60); kbc_wait_in(); outb(0x60, st);
    mouse_found = mouse_cmd(0xF6) && mouse_cmd(0xF4);
}

/* ===================== time, RTC, reboot, shutdown ===================== */
u32 uptime_secs(void) { return sys_ticks / 100; }
void sleep_ticks(u32 n) { u32 t = sys_ticks; while (sys_ticks - t < n) __asm__ volatile("hlt"); }
static u8 cmos(u8 r) { outb(0x70, r); return inb(0x71); }
void rtc_read(int *h, int *m, int *s) {
    for (int i = 0; i < 10000 && (cmos(0x0A) & 0x80); i++) {}
    u8 sec = cmos(0), min = cmos(2), hr = cmos(4), b = cmos(0x0B);
    int pm = hr & 0x80; hr &= 0x7F;
    if (!(b & 4)) { sec = (sec & 15) + (sec >> 4) * 10; min = (min & 15) + (min >> 4) * 10; hr = (hr & 15) + (hr >> 4) * 10; }
    if (!(b & 2)) { hr %= 12; if (pm) hr += 12; }
    *h = hr % 24; *m = min % 60; *s = sec % 60;
}
void rtc_date(int *y, int *mo, int *d) {
    for (int i = 0; i < 10000 && (cmos(0x0A) & 0x80); i++) {}
    u8 dd = cmos(7), mm = cmos(8), yy = cmos(9), b = cmos(0x0B);
    if (!(b & 4)) { dd = (dd & 15) + (dd >> 4) * 10; mm = (mm & 15) + (mm >> 4) * 10; yy = (yy & 15) + (yy >> 4) * 10; }
    *y = 2000 + yy; *mo = mm; *d = dd;
}
u32 read_cr0(void) { u32 v; __asm__ volatile("mov %%cr0,%0" : "=r"(v)); return v; }
void sys_reboot(void) {
    __asm__ volatile("cli");
    for (int i = 0; i < 100000 && (inb(0x64) & 2); i++) {}
    outb(0x64, 0xFE);
    struct { u16 l; u32 b; } __attribute__((packed)) nul = { 0, 0 };
    __asm__ volatile("lidt %0; int3" : : "m"(nul));     /* triple fault fallback */
    for (;;) __asm__ volatile("hlt");
}
void sys_shutdown(void) {
    __asm__ volatile("cli");
    if (cur_res) gfx_set_res(0);
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
    gfx_rect(0, 0, SCR_W, SCR_H, C_BLACK);
    gfx_text(40, 92, "It's now safe to turn off", C_LRED);
    gfx_text(88, 102, "your computer.", C_LRED);
    gfx_flip();
    outw(0x604, 0x2000); outw(0xB004, 0x2000); outw(0x4004, 0x3400);   /* QEMU/Bochs/VBox ACPI off */
    for (;;) __asm__ volatile("hlt");
}

/* ===================== BSOD ===================== */
static const char *const exc_names[20] = { "DIVIDE BY ZERO", "DEBUG", "NMI", "BREAKPOINT", "OVERFLOW", "BOUND RANGE",
    "INVALID OPCODE", "DEVICE NOT AVAILABLE", "DOUBLE FAULT", "COPROCESSOR OVERRUN", "INVALID TSS", "SEGMENT NOT PRESENT",
    "STACK FAULT", "GENERAL PROTECTION FAULT", "PAGE FAULT", "RESERVED", "X87 FPU ERROR", "ALIGNMENT CHECK", "MACHINE CHECK", "SIMD FP ERROR" };
static void bsod_line(int row, const char *l, const char *a, u32 av, const char *l2, u32 bv, int two) {
    char b[48], *p = b;
    p = fmt_str(p, l); p = fmt_hex(p, av, 8);
    if (two) { p = fmt_str(p, "  "); p = fmt_str(p, l2); fmt_hex(p, bv, 8); }
    (void)a;
    gfx_text(8, 52 + row * 10, b, C_WHITE);
}
static const char *bsod_detail = "";
static int gfx_set_res(int r);
static void bsod_core(const Regs *r, const char *name, u32 cr2) {
    __asm__ volatile("cli");
    if (cur_res) gfx_set_res(0);
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
    gfx_rect(0, 0, SCR_W, SCR_H, C_BLUE);
    gfx_rect(120, 8, 80, 10, C_LGRAY);
    gfx_text(132, 9, "CCORE 0.2", C_BLUE);
    gfx_text(8, 26, "A fatal exception has occurred.", C_WHITE);
    char b[48], *p;
    if (r->int_no >= 0xFE) { p = fmt_str(b, "Stop: "); fmt_str(p, name); }
    else { p = fmt_str(b, "Exception "); p = fmt_hex(p, r->int_no, 2); p = fmt_str(p, ": "); fmt_str(p, name); }
    gfx_text(8, 36, b, C_YELLOW);
    gfx_text(8, 44, bsod_detail, C_LGRAY);
    bsod_line(0, "EAX=", NULL, r->eax, "EBX=", r->ebx, 1);
    bsod_line(1, "ECX=", NULL, r->ecx, "EDX=", r->edx, 1);
    bsod_line(2, "ESI=", NULL, r->esi, "EDI=", r->edi, 1);
    bsod_line(3, "EBP=", NULL, r->ebp, "ESP=", r->esp_d, 1);
    bsod_line(4, "EIP=", NULL, r->eip, "CS =", r->cs, 1);
    bsod_line(5, "EFLAGS=", NULL, r->eflags, "ERR=", r->err_code, 1);
    if (r->int_no == 14) bsod_line(6, "CR2=", NULL, cr2, NULL, 0, 0);
    gfx_text(8, 130, "The system has been halted to prevent", C_WHITE);
    gfx_text(8, 140, "damage. Unsaved data may be lost.", C_WHITE);
    gfx_text(8, 164, "Press any key to restart your computer", C_WHITE);
    gfx_flip();
    while (inb(0x64) & 1) inb(0x60);
    for (;;) { u8 st = inb(0x64); if ((st & 1) && !(st & 0x20)) { if (!(inb(0x60) & 0x80)) break; } else if (st & 1) inb(0x60); }
    sys_reboot();
}
void bsod_stop(const char *name, const char *detail) {
    Regs r; memset(&r, 0, sizeof r);
    __asm__ volatile("movl %%eax,%0; movl %%ebx,%1; movl %%ecx,%2; movl %%edx,%3"
                     : "=m"(r.eax), "=m"(r.ebx), "=m"(r.ecx), "=m"(r.edx));
    __asm__ volatile("movl %%esi,%0; movl %%edi,%1; movl %%ebp,%2; movl %%esp,%3"
                     : "=m"(r.esi), "=m"(r.edi), "=m"(r.ebp), "=m"(r.esp_d));
    __asm__ volatile("movl %%cs,%0; pushfl; popl %1" : "=r"(r.cs), "=r"(r.eflags));
    r.eip = (u32)__builtin_return_address(0); r.int_no = 0xFE; r.err_code = 0;
    bsod_detail = detail;
    bsod_core(&r, name, 0);
}
void bsod_manual(void) { bsod_stop("MANUALLY_INITIATED_CRASH", "Crash requested by the user."); }
void isr_dispatch(Regs *r) {
    if (r->int_no < 32) {
        u32 cr2; __asm__ volatile("mov %%cr2,%0" : "=r"(cr2));
        bsod_core(r, r->int_no < 20 ? exc_names[r->int_no] : "RESERVED", cr2);
        return;
    }
    switch (r->int_no - 32) {
    case 0: sys_ticks++; break;
    case 1: irq_keyboard(); break;
    case 12: irq_mouse(); break;
    }
    if (r->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

/* ===================== window manager ===================== */
Window wins[MAX_WINDOWS];
int zorder[MAX_WINDOWS], zcount;
static int next_pid = 1;
static int cap_win = -1, drag_win = -1, drag_dx, drag_dy;

Window *win_create(const char *title, int app, int x, int y, int cw, int ch, u32 mem_kb) {
    for (int i = 0; i < MAX_WINDOWS; i++)
        if (!wins[i].used) {
            Window *w = &wins[i];
            memset(w, 0, sizeof *w);
            w->used = 1; w->pid = next_pid++; w->app = app; w->cw = cw; w->ch = ch;
            int ow = cw + 2 * BORDER, oh = ch + CLI_Y + BORDER;
            if (x + ow > SCR_W) x = SCR_W - ow;
            if (y + oh > DESK_H) y = DESK_H - oh;
            w->x = x < 0 ? 0 : x; w->y = y < 0 ? 0 : y;
            w->bg = C_LGRAY; w->mem_kb = mem_kb; w->start_tick = sys_ticks;
            str_ncpy(w->title, title, 32);
            zorder[zcount++] = i;
            return w;
        }
    return NULL;
}
Window *win_find(int app) { for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used && wins[i].app == app) return &wins[i]; return NULL; }
Window *win_by_pid(int pid) { for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used && wins[i].pid == pid) return &wins[i]; return NULL; }
Window *win_top(void) {
    for (int z = zcount - 1; z >= 0; z--) if (!wins[zorder[z]].minimized) return &wins[zorder[z]];
    return NULL;
}
static void z_remove(int idx) {
    int j = 0;
    for (int z = 0; z < zcount; z++) if (zorder[z] != idx) zorder[j++] = zorder[z];
    zcount = j;
}
void win_focus(Window *w) {
    int idx = w - wins;
    w->minimized = 0;
    z_remove(idx); zorder[zcount++] = idx;
}
void win_close(Window *w) {
    int idx = w - wins;
    if (!w->used) return;
    if (w->onclose) w->onclose(w);
    z_remove(idx);
    if (cap_win == idx) cap_win = -1;
    if (drag_win == idx) drag_win = -1;
    w->used = 0;
}
static void win_toggle_max(Window *w) {
    if (!w->maximized) {
        w->rx = w->x; w->ry = w->y; w->rcw = w->cw; w->rch = w->ch;
        w->x = 0; w->y = 0; w->cw = SCR_W - 2 * BORDER; w->ch = DESK_H - CLI_Y - BORDER; w->maximized = 1;
    } else {
        w->x = w->rx; w->y = w->ry; w->cw = w->rcw; w->ch = w->rch; w->maximized = 0;
    }
}
int dwm_set_resolution(int r) {
    if (gfx_set_res(r)) return -1;
    cfg.res = r;
    wallpaper_rebuild();
    for (int i = 0; i < NICONS; i++) {
        if (icon_x[i] > SCR_W - 56) icon_x[i] = SCR_W - 56;
        if (icon_y[i] > DESK_H - 30) icon_y[i] = DESK_H - 30;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) {
        Window *w = &wins[i];
        if (!w->used) continue;
        if (w->maximized) { w->x = 0; w->y = 0; w->cw = SCR_W - 2 * BORDER; w->ch = DESK_H - CLI_Y - BORDER; }
        int ow = w->cw + 2 * BORDER, oh = w->ch + CLI_Y + BORDER;
        if (w->x + ow > SCR_W) w->x = SCR_W - ow;
        if (w->y + oh > DESK_H) w->y = DESK_H - oh;
        if (w->x < 0) w->x = 0;
        if (w->y < 0) w->y = 0;
    }
    return 0;
}
static void win_draw(Window *w, int focused) {
    int ow = w->cw + 2 * BORDER, oh = w->ch + CLI_Y + BORDER, x = w->x, y = w->y;
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, DESK_H);
    gfx_frame(x, y, ow, oh, C_BLACK);
    gfx_bevel(x + 1, y + 1, ow - 2, oh - 2, 1);
    int tx = x + BORDER, ty = y + BORDER;
    u8 tb = focused ? theme_active[cfg.theme] : C_WHITE, tc = focused ? theme_text[cfg.theme] : C_BLACK;
    gfx_rect(tx, ty, w->cw, TITLE_H, tb);
    gfx_hline(tx, ty + TITLE_H, w->cw, C_BLACK);
    int maxc = (w->cw - 3 * TITLE_H) / 8, tl = strlen(w->title); if (tl > maxc) tl = maxc;
    gfx_text_n(tx + (w->cw - tl * 8) / 2, ty + 1, w->title, tc, tl);
    gfx_bevel(tx, ty, TITLE_H, TITLE_H, !btn_hot(tx, ty, TITLE_H, TITLE_H));   /* control (close) box */
    gfx_rect(tx + 2, ty + 4, TITLE_H - 4, 2, C_DGRAY); gfx_hline(tx + 2, ty + 3, TITLE_H - 4, C_BLACK);
    int mx = tx + w->cw - TITLE_H, nx = mx - TITLE_H;
    gfx_bevel(mx, ty, TITLE_H, TITLE_H, !btn_hot(mx, ty, TITLE_H, TITLE_H));
    for (int i = 0; i < 3; i++) gfx_hline(mx + 4 - i, ty + 3 + i, 1 + 2 * i, C_BLACK);          /* ^ maximize */
    if (w->maximized) for (int i = 0; i < 2; i++) gfx_hline(mx + 4 - i, ty + 6 + i, 1 + 2 * i, C_BLACK);
    gfx_bevel(nx, ty, TITLE_H, TITLE_H, !btn_hot(nx, ty, TITLE_H, TITLE_H));
    for (int i = 0; i < 3; i++) gfx_hline(nx + 2 + i, ty + 3 + i, 5 - 2 * i, C_BLACK);          /* v minimize */
    int cx = x + CLI_X, cy = y + CLI_Y;
    int x1 = cx + w->cw, y1 = cy + w->ch; if (y1 > DESK_H) y1 = DESK_H;
    gfx_set_clip(cx, cy, x1, y1);
    gfx_rect(cx, cy, w->cw, w->ch, w->bg);
    gfx_set_origin(cx, cy);
    if (w->draw) w->draw(w);
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
}
static int win_at(int px, int py) {
    for (int z = zcount - 1; z >= 0; z--) {
        Window *w = &wins[zorder[z]];
        if (!w->minimized && ui_in(px, py, w->x, w->y, w->cw + 2 * BORDER, w->ch + CLI_Y + BORDER)) return zorder[z];
    }
    return -1;
}

/* generic message box */
static char msgtext[40];
static void msg_draw(Window *w) { gfx_text(8, 8, msgtext, C_BLACK); gfx_button((w->cw - 40) / 2, 26, 40, 14, "OK", 0); }
static void msg_mouse(Window *w, int t, int x, int y, int b) { (void)b; if (t == MOUSE_DOWN && ui_in(x, y, (w->cw - 40) / 2, 26, 40, 14)) win_close(w); }
static void msg_key(Window *w, int k) { if (k == KEY_ENTER || k == KEY_ESC) win_close(w); }
void dwm_message(const char *text) {
    str_ncpy(msgtext, text, 40);
    int cw = strlen(msgtext) * 8 + 16;
    if (cw < 100) cw = 100;
    if (cw > 300) cw = 300;
    Window *w = win_find(APP_MSG);
    if (w) w->cw = cw;
    if (!w) {
        w = win_create("CCORE", APP_MSG, (SCR_W - cw - 2 * BORDER) / 2, 60, cw, 46, app_mem_kb(sizeof msgtext));
        if (!w) return;
        w->draw = msg_draw; w->mouse = msg_mouse; w->key = msg_key;
    }
    win_focus(w);
}

/* ===================== desktop: icons, menus, taskbar, cursor ===================== */
typedef struct { const char *label; int act; } MenuItem;
enum { ACT_NONE, ACT_CALC, ACT_NOTEPAD, ACT_PAINT, ACT_FILES, ACT_SETTINGS, ACT_TASKMAN, ACT_ABOUT, ACT_LOGOFF, ACT_RESTART, ACT_SHUTDOWN, ACT_ICON_OPEN, ACT_ICON_RENAME, ACT_ARRANGE, ACT_CMD, ACT_CMDADMIN };
static const MenuItem start_items[] = { {"Calculator", ACT_CALC}, {"Notepad", ACT_NOTEPAD}, {"Paint", ACT_PAINT}, {"File Manager", ACT_FILES},
    {"Control Panel", ACT_SETTINGS}, {"Task Manager", ACT_TASKMAN}, {"About CCORE", ACT_ABOUT}, {"Command Prompt", ACT_CMD},
    {"Admin Prompt", ACT_CMDADMIN}, {"-", 0}, {"Log Off", ACT_LOGOFF},
    {"Restart", ACT_RESTART}, {"Shut Down", ACT_SHUTDOWN} };
static const MenuItem desk_items[] = { {"Control Panel", ACT_SETTINGS}, {"Task Manager", ACT_TASKMAN}, {"About CCORE", ACT_ABOUT}, {"Arrange Icons", ACT_ARRANGE} };
static const MenuItem icon_items[] = { {"Open", ACT_ICON_OPEN}, {"Rename", ACT_ICON_RENAME} };
#define START_W 54
#define TB_BH 12
static const MenuItem *menu_items; static int menu_n, menu_x, menu_y, menu_w = 128;
static int logoff, sel_icon = -1, last_click_id = -1; static u32 last_click_tick;
static const int icon_acts[NICONS] = { ACT_CALC, ACT_NOTEPAD, ACT_PAINT, ACT_FILES, ACT_SETTINGS, ACT_TASKMAN, ACT_ABOUT, ACT_CMD };
static int ctx_icon, rename_icon = -1, icon_drag = -1, icon_dx, icon_dy, icon_sx, icon_sy, icon_moved;

static void menu_open(const MenuItem *it, int n, int x, int y, int bottom) {
    menu_items = it; menu_n = n; menu_w = 128;
    int h = n * 10 + 4;
    menu_x = x; menu_y = bottom ? y - h : y;
    if (menu_x + menu_w > SCR_W) menu_x = SCR_W - menu_w;
    if (menu_y + h > DESK_H) menu_y = DESK_H - h;
}
static int menu_hit(int px, int py) {
    if (!menu_items || !ui_in(px, py, menu_x, menu_y, menu_w, menu_n * 10 + 4)) return -1;
    int i = (py - menu_y - 2) / 10;
    return (i >= 0 && i < menu_n && menu_items[i].act) ? i : -1;
}
static void launch(int act) {
    switch (act) {
    case ACT_CALC: calc_launch(); break; case ACT_NOTEPAD: notepad_launch(); break; case ACT_PAINT: paint_launch(); break;
    case ACT_FILES: filemgr_launch(); break; case ACT_SETTINGS: settings_launch(); break; case ACT_TASKMAN: taskman_launch(); break;
    case ACT_ABOUT: cver_launch(); break; case ACT_LOGOFF: logoff = 1; break; case ACT_RESTART: sys_reboot(); break;
    case ACT_SHUTDOWN: sys_shutdown(); break;
    case ACT_ICON_OPEN: launch(icon_acts[ctx_icon]); break;
    case ACT_ICON_RENAME: rename_icon = ctx_icon; break;
    case ACT_ARRANGE: icons_arrange(); settings_save(); break;
    case ACT_CMD: cmd_launch(0); break;
    case ACT_CMDADMIN: cmd_launch(1); break;
    }
}
static void draw_icon_art(int act, int x, int y) {
    switch (act) {
    case ACT_CALC: gfx_rect(x + 2, y, 12, 16, C_LGRAY); gfx_frame(x + 2, y, 12, 16, C_BLACK); gfx_rect(x + 4, y + 2, 8, 3, C_LGREEN);
        for (int j = 0; j < 3; j++) { for (int i = 0; i < 3; i++) gfx_rect(x + 4 + i * 3, y + 7 + j * 3, 2, 2, C_BLACK); }
        break;
    case ACT_NOTEPAD: gfx_rect(x + 2, y + 1, 12, 15, C_WHITE); gfx_frame(x + 2, y + 1, 12, 15, C_BLACK); gfx_rect(x + 2, y, 12, 3, C_RED);
        for (int j = 0; j < 4; j++) { gfx_hline(x + 4, y + 6 + j * 2, 8, C_BLUE); }
        break;
    case ACT_PAINT: gfx_rect(x + 1, y + 2, 14, 12, C_WHITE); gfx_frame(x + 1, y + 2, 14, 12, C_BLACK); gfx_rect(x + 3, y + 4, 4, 4, C_RED);
        gfx_rect(x + 8, y + 5, 4, 4, C_BLUE); gfx_rect(x + 5, y + 9, 5, 3, C_GREEN); break;
    case ACT_FILES: gfx_rect(x + 1, y + 3, 6, 3, C_YELLOW); gfx_rect(x + 1, y + 5, 14, 10, C_YELLOW); gfx_frame(x + 1, y + 5, 14, 10, C_BLACK); break;
    case ACT_SETTINGS: gfx_rect(x + 1, y + 2, 14, 12, C_LGRAY); gfx_frame(x + 1, y + 2, 14, 12, C_BLACK);
        gfx_hline(x + 3, y + 5, 10, C_BLACK); gfx_rect(x + 5, y + 4, 2, 3, C_RED); gfx_hline(x + 3, y + 9, 10, C_BLACK); gfx_rect(x + 9, y + 8, 2, 3, C_BLUE); break;
    case ACT_TASKMAN: gfx_rect(x + 1, y + 2, 14, 12, C_WHITE); gfx_frame(x + 1, y + 2, 14, 12, C_BLACK);
        gfx_rect(x + 3, y + 9, 2, 3, C_GREEN); gfx_rect(x + 7, y + 6, 2, 6, C_GREEN); gfx_rect(x + 11, y + 4, 2, 8, C_GREEN); break;
    case ACT_CMD: gfx_rect(x + 1, y + 2, 14, 12, C_BLACK); gfx_frame(x + 1, y + 2, 14, 12, C_LGRAY); gfx_text(x + 3, y + 4, ">", C_LGRAY); gfx_hline(x + 8, y + 10, 4, C_LGRAY); break;
    default: gfx_rect(x + 1, y + 1, 14, 14, C_BLUE); gfx_frame(x + 1, y + 1, 14, 14, C_BLACK); gfx_text(x + 4, y + 4, "C", C_WHITE); break;
    }
}
static void draw_icons(void) {
    for (int i = 0; i < NICONS; i++) {
        int tw = strlen(icon_name[i]) * 8, lx = icon_x[i] + (56 - tw) / 2, ly = icon_y[i] + 18;
        draw_icon_art(icon_acts[i], icon_x[i] + 20, icon_y[i]);
        if (rename_icon == i) {
            int bw = tw + 10, bx = icon_x[i] + (56 - bw) / 2;
            gfx_rect(bx, ly - 1, bw, 10, C_WHITE); gfx_frame(bx, ly - 1, bw, 10, C_BLACK);
            gfx_text(bx + 2, ly, icon_name[i], C_BLACK);
            if ((sys_ticks / 50) & 1) gfx_vline(bx + 2 + tw, ly, 8, C_BLACK);
            continue;
        }
        if (i == sel_icon) gfx_rect(lx - 1, ly, tw + 1, 9, C_BLUE);
        else gfx_text(lx + 1, ly + 1, icon_name[i], C_BLACK);
        gfx_text(lx, ly, icon_name[i], C_WHITE);
    }
}
static void rename_end(void) {
    if (rename_icon < 0) return;
    if (!icon_name[rename_icon][0]) str_ncpy(icon_name[rename_icon], icon_def_name[rename_icon], 9);
    rename_icon = -1; settings_save();
}
static void rename_key(int k) {
    char *n = icon_name[rename_icon]; int l = strlen(n);
    if (k == KEY_ENTER || k == KEY_ESC) rename_end();
    else if (k == KEY_BS) { if (l) n[l - 1] = 0; }
    else if (k > 32 && k < 127 && l < 7) { n[l] = k; n[l + 1] = 0; }
}
static int tb_layout(int *idx, int *xs, int *ws) {
    int n = 0;
    for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) idx[n++] = i;
    if (!n) return 0;
    int bw = (SCR_W - START_W - 82) / n; if (bw > 64) bw = 64;
    for (int k = 0; k < n; k++) { xs[k] = START_W + 6 + k * bw; ws[k] = bw - 2; }
    return n;
}
static char clock_str[9];
static u32 clock_sec = 0xFFFFFFFF;
static void draw_taskbar(void) {
    gfx_rect(0, DESK_H, SCR_W, TASKBAR_H, C_LGRAY);
    gfx_hline(0, DESK_H, SCR_W, C_BLACK);
    gfx_button(2, DESK_H + 2, START_W, TB_BH, "Start", menu_items == start_items);
    int idx[MAX_WINDOWS], xs[MAX_WINDOWS], ws[MAX_WINDOWS], n = tb_layout(idx, xs, ws);
    Window *top = win_top();
    for (int k = 0; k < n; k++) {
        Window *w = &wins[idx[k]];
        gfx_button(xs[k], DESK_H + 2, ws[k], TB_BH, "", w == top);
        gfx_text_n(xs[k] + 3, DESK_H + 4, w->title, C_BLACK, (ws[k] - 4) / 8);
    }
    if (clock_sec != sys_ticks / 100) {
        clock_sec = sys_ticks / 100;
        int h, m, s; rtc_read(&h, &m, &s);
        char *p = fmt_2d(clock_str, h); *p++ = ':'; p = fmt_2d(p, m); *p++ = ':'; fmt_2d(p, s);
    }
    gfx_sunken(SCR_W - 70, DESK_H + 2, 68, TB_BH, C_LGRAY);
    gfx_text(SCR_W - 67, DESK_H + 4, clock_str, C_BLACK);
}
static void draw_menu(void) {
    if (!menu_items) return;
    gfx_rect(menu_x, menu_y, menu_w, menu_n * 10 + 4, C_BLACK);
    gfx_bevel(menu_x + 1, menu_y + 1, menu_w - 2, menu_n * 10 + 2, 1);
    int hi = menu_hit(mouse_x, mouse_y);
    for (int i = 0; i < menu_n; i++) {
        int y = menu_y + 2 + i * 10;
        if (!menu_items[i].act) { gfx_hline(menu_x + 3, y + 5, menu_w - 6, C_DGRAY); continue; }
        if (i == hi) gfx_rect(menu_x + 2, y, menu_w - 4, 10, C_BLUE);
        gfx_text(menu_x + 6, y + 1, menu_items[i].label, i == hi ? C_WHITE : C_BLACK);
    }
}
static const char *const cur_spr[14] = { "X         ", "XX        ", "X.X       ", "X..X      ", "X...X     ", "X....X    ", "X.....X   ",
    "X......X  ", "X...XXXXX ", "X.X.X     ", "XX X.X    ", "X  X.X    ", "    X.X   ", "    XX    " };
static void draw_cursor_at(int x, int y, int mono) {
    for (int j = 0; j < 14; j++)
        for (int i = 0; i < 10; i++) {
            char c = cur_spr[j][i];
            if (c == 'X') gfx_pixel(x + i, y + j, mono ? mono : C_BLACK);
            else if (c == '.') gfx_pixel(x + i, y + j, mono ? mono : C_WHITE);
        }
}
static struct { int x, y; } trail[16];
static int trail_head;
void draw_mouse(void) {
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
    trail[trail_head].x = mouse_x; trail[trail_head].y = mouse_y; trail_head = (trail_head + 1) & 15;
    if (cfg.trails) {
        int len = cfg.trail_len;
        for (int age = len; age >= 1; age--) {
            int i = (trail_head - 1 - age) & 15;
            draw_cursor_at(trail[i].x, trail[i].y, 0);                              /* same look, no darkening */
        }
    }
    draw_cursor_at(mouse_x, mouse_y, 0);
}
static void render_frame(void) {
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
    memcpy(backbuf, wallbuf, SCR_W * SCR_H);
    gfx_set_clip(0, 0, SCR_W, DESK_H);
    draw_icons();
    Window *top = win_top();
    for (int z = 0; z < zcount; z++) { Window *w = &wins[zorder[z]]; if (!w->minimized) win_draw(w, w == top); }
    gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H);
    draw_taskbar();
    draw_menu();
    draw_mouse();
    gfx_flip();
}
static void handle_mouse(MouseEvt *e) {
    if (e->type == MOUSE_MOVE) {
        if (icon_drag >= 0) {
            int ax = e->x - icon_sx, ay = e->y - icon_sy;
            if ((ax < 0 ? -ax : ax) + (ay < 0 ? -ay : ay) > 3) icon_moved = 1;
            if (icon_moved) {
                int nx = e->x - icon_dx, ny = e->y - icon_dy;
                if (nx < 0) nx = 0;
                if (nx > SCR_W - 56) nx = SCR_W - 56;
                if (ny < 0) ny = 0;
                if (ny > DESK_H - 30) ny = DESK_H - 30;
                icon_x[icon_drag] = nx; icon_y[icon_drag] = ny;
            }
            return;
        }
        if (drag_win >= 0) {
            Window *w = &wins[drag_win]; int ow = w->cw + 2 * BORDER;
            w->x = e->x - drag_dx; w->y = e->y - drag_dy;
            if (w->x < 20 - ow) w->x = 20 - ow;
            if (w->x > SCR_W - 20) w->x = SCR_W - 20;
            if (w->y < 0) w->y = 0;
            if (w->y > DESK_H - 12) w->y = DESK_H - 12;
        } else if (cap_win >= 0) {
            Window *w = &wins[cap_win];
            if (w->used && w->mouse) w->mouse(w, MOUSE_MOVE, e->x - w->x - CLI_X, e->y - w->y - CLI_Y, e->btn);
        }
        return;
    }
    if (e->type == MOUSE_UP) {
        if (icon_drag >= 0) {
            if (icon_moved) {
                icon_x[icon_drag] = (icon_x[icon_drag] + 2) / 4 * 4; icon_y[icon_drag] = (icon_y[icon_drag] + 2) / 4 * 4;
                last_click_id = -1; settings_save();
            }
            icon_drag = -1;
            return;
        }
        if (drag_win >= 0) { drag_win = -1; return; }
        if (cap_win >= 0) {
            int ci = cap_win; cap_win = -1;
            Window *w = &wins[ci];
            if (w->used && w->mouse) w->mouse(w, MOUSE_UP, e->x - w->x - CLI_X, e->y - w->y - CLI_Y, e->btn);
        }
        return;
    }
    /* button down */
    rename_end();
    if (menu_items) {
        int hi = menu_hit(e->x, e->y), act = hi >= 0 ? menu_items[hi].act : 0;
        int on_start = ui_in(e->x, e->y, 2, DESK_H + 2, START_W, TB_BH);
        menu_items = NULL;
        if (act) launch(act);
        (void)on_start;
        return;
    }
    if (e->y >= DESK_H) {
        if (e->btn != BTN_L) return;
        if (ui_in(e->x, e->y, 2, DESK_H + 2, START_W, TB_BH)) { menu_open(start_items, 13, 2, DESK_H, 1); return; }
        int idx[MAX_WINDOWS], xs[MAX_WINDOWS], ws[MAX_WINDOWS], n = tb_layout(idx, xs, ws);
        for (int k = 0; k < n; k++)
            if (ui_in(e->x, e->y, xs[k], DESK_H + 2, ws[k], TB_BH)) {
                Window *w = &wins[idx[k]];
                if (w->minimized) win_focus(w);
                else if (w == win_top()) w->minimized = 1;
                else win_focus(w);
                return;
            }
        return;
    }
    int wi = win_at(e->x, e->y);
    if (wi >= 0) {
        Window *w = &wins[wi];
        if (w != win_top()) win_focus(w);
        int lx = e->x - w->x, ly = e->y - w->y;
        if (ly >= BORDER && ly < BORDER + TITLE_H && lx >= BORDER && lx < BORDER + w->cw) {
            if (e->btn != BTN_L) return;
            if (lx < BORDER + TITLE_H) { win_close(w); return; }
            if (lx >= BORDER + w->cw - TITLE_H) { win_toggle_max(w); return; }
            if (lx >= BORDER + w->cw - 2 * TITLE_H) { w->minimized = 1; return; }
            if (last_click_id == 100 + wi && sys_ticks - last_click_tick < 40) { win_toggle_max(w); last_click_id = -1; return; }
            last_click_id = 100 + wi; last_click_tick = sys_ticks;
            if (!w->maximized) { drag_win = wi; drag_dx = e->x - w->x; drag_dy = e->y - w->y; }
        } else if (ly >= CLI_Y && lx >= CLI_X && lx < CLI_X + w->cw && ly < CLI_Y + w->ch) {
            cap_win = wi;
            if (w->mouse) w->mouse(w, MOUSE_DOWN, lx - CLI_X, ly - CLI_Y, e->btn);
        }
        return;
    }
    int hit = -1;
    for (int i = 0; i < NICONS; i++) if (ui_in(e->x, e->y, icon_x[i], icon_y[i], 56, 28)) hit = i;
    if (e->btn == BTN_R) {
        if (hit >= 0) { sel_icon = ctx_icon = hit; menu_open(icon_items, 2, e->x, e->y, 0); }
        else menu_open(desk_items, 4, e->x, e->y, 0);
        return;
    }
    sel_icon = hit;
    if (hit < 0) { last_click_id = -1; return; }
    if (last_click_id == hit && sys_ticks - last_click_tick < 50) { last_click_id = -1; launch(icon_acts[hit]); return; }
    last_click_id = hit; last_click_tick = sys_ticks;
    icon_drag = hit; icon_dx = e->x - icon_x[hit]; icon_dy = e->y - icon_y[hit]; icon_sx = e->x; icon_sy = e->y; icon_moved = 0;
}
__attribute__((unused)) static void desktop_run(void) {
    logoff = 0; menu_items = NULL; cap_win = drag_win = icon_drag = -1; sel_icon = -1; rename_icon = -1;
    while (kbd_get() >= 0) {}
    u32 last = sys_ticks;
    while (!logoff) {
        MouseEvt e; int k;
        while (mouse_next(&e)) handle_mouse(&e);
        while ((k = kbd_get()) >= 0) {
            if (rename_icon >= 0) { rename_key(k); continue; }
            if (menu_items) { if (k == KEY_ESC) menu_items = NULL; continue; }
            Window *t = win_top();
            if (t && t->key) t->key(t, k);
        }
        for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used && wins[i].tick) wins[i].tick(&wins[i]);
        render_frame();
        while (sys_ticks - last < 2) __asm__ volatile("hlt");
        last = sys_ticks;
    }
    for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) win_close(&wins[i]);
}

/* ===================== boot: POST, loading screen, login ===================== */
static int post_y;
void gfx_reset(void) { gfx_set_origin(0, 0); gfx_set_clip(0, 0, SCR_W, SCR_H); }
static void post_print(const char *s, u8 c) { gfx_text(4, post_y, s, c); post_y += 9; gfx_flip(); sleep_ticks(4); }
static int mem_ok(u32 a) {
    volatile u32 *p = (volatile u32 *)a; u32 sv = *p; int ok = 1;
    *p = 0xA5A5A5A5; if (*p != 0xA5A5A5A5) ok = 0;
    *p = 0x5A5A5A5A; if (*p != 0x5A5A5A5A) ok = 0;
    *p = sv;
    return ok;
}
static void cpu_detect(void) {
    u32 a, b, c, d, r[12];
    cpu_ok = 0; cpu_name[0] = 0;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000000u));
    if (a < 0x80000004u) return;
    for (int i = 0; i < 3; i++)
        __asm__ volatile("cpuid" : "=a"(r[i * 4]), "=b"(r[i * 4 + 1]), "=c"(r[i * 4 + 2]), "=d"(r[i * 4 + 3]) : "a"(0x80000002u + i));
    memcpy(cpu_name, r, 48); cpu_name[48] = 0;
    int st = 0; while (cpu_name[st] == ' ') st++;
    if (st) memmove(cpu_name, cpu_name + st, strlen(cpu_name + st) + 1);
    int n = strlen(cpu_name); while (n > 0 && cpu_name[n - 1] == ' ') cpu_name[--n] = 0;
    cpu_ok = n > 0;
}
static void post_screen(void) {
    gfx_reset(); gfx_rect(0, 0, SCR_W, SCR_H, C_BLACK); post_y = 4;
    post_print("CCORE BIOS v0.2", C_WHITE);
    cpu_detect();
    char buf[64], *p;
    if (cpu_ok) {
        p = fmt_str(buf, "CPU: "); str_ncpy(p, cpu_name, 34); post_print(buf, C_LGRAY);
        if (strlen(cpu_name) > 33) { p = fmt_str(buf, "     "); str_ncpy(p, cpu_name + 33, 16); post_print(buf, C_LGRAY); }
    } else post_print("CPU: Not detected", C_LRED);
    post_y += 4;
    int my = post_y; post_y += 13;
    u32 mb = 1;
    for (; mb < 512; mb++) {
        if (!mem_ok(mb << 20) || !mem_ok((mb << 20) + 0xFFFFC)) break;
        if (mb % 2 == 0 || mb < 8) {
            gfx_rect(4, my, 300, 8, C_BLACK);
            p = fmt_str(buf, "Memory Test: "); p = fmt_u(p, (mb + 1) * 1024); fmt_str(p, " KB");
            gfx_text(4, my, buf, C_WHITE); gfx_flip(); sleep_ticks(1);
        }
    }
    ram_kb = mb * 1024;
    gfx_rect(4, my, 300, 8, C_BLACK);
    p = fmt_str(buf, "Memory Test: "); p = fmt_u(p, ram_kb); fmt_str(p, " KB OK");
    gfx_text(4, my, buf, C_LGREEN); gfx_flip(); sleep_ticks(10);
    ata_init();
    if (ata_ok) { p = fmt_str(buf, "ATA0: "); str_ncpy(p, ata_model, 30); post_print(buf, C_LGRAY);
        p = fmt_u(buf, ata_sectors / 2048); fmt_str(p, " MB, LBA28 PIO"); post_print(buf, C_LGRAY); }
    else post_print("ATA0: no drive detected", C_LRED);
    post_print("Keyboard: PS/2 detected", C_LGRAY);
    post_print(mouse_found ? "Mouse: PS/2 detected" : "Mouse: not found", mouse_found ? C_LGRAY : C_LRED);
    post_y += 4; post_print("Booting CCORE from hard disk...", C_WHITE);
    sleep_ticks(10);
}
static void loading_screen(void) {
    u32 t0 = sys_ticks; int stage = 0;
    const char *msg[5] = { "Starting CCORE...", "Mounting ATA disk...", "Loading settings...", "Starting window manager...", "Preparing desktop..." };
    for (;;) {
        u32 el = sys_ticks - t0; int pct = el * 100 / 100; if (pct > 100) pct = 100;
        int want = pct / 20; if (want > 4) want = 4;
        while (stage < want) {
            stage++;
            if (stage == 1) fs_mount();
            if (stage == 2) { wallpaper_rebuild(); }
        }
        gfx_reset(); gfx_rect(0, 0, SCR_W, SCR_H, C_BLUE);
        for (int i = 0; i < SCR_H; i += 4) gfx_hline(0, i, SCR_W, C_LBLUE);
        gfx_rect(40, 28, 240, 60, C_BLUE); gfx_frame(40, 28, 240, 60, C_WHITE);
        gfx_text_big(66 + 3, 38 + 3, "CCORE", C_BLACK, 5); gfx_text_big(66, 38, "CCORE", C_WHITE, 5);
        gfx_text(100, 78, "Version 0.2", C_YELLOW);
        { const char *m = msg[stage < 5 ? stage : 4]; gfx_text((SCR_W - (int)strlen(m) * 8) / 2, 112, m, C_WHITE); }
        gfx_sunken(60, 124, 200, 14, C_LGRAY);
        for (int i = 0; i < pct * 192 / 100 / 8; i++) gfx_rect(64 + i * 8, 127, 7, 8, C_BLUE);
        gfx_flip();
        if (pct >= 100) break;
        sleep_ticks(2);
    }
    if (stage < 2) { if (!fs_ok) fs_mount(); wallpaper_rebuild(); }
    sleep_ticks(5);
}
void gfx_wallpaper(void) { memcpy(backbuf, wallbuf, SCR_W * SCR_H); }
#ifndef SETUP_BUILD
static void login_screen(void) {
    char user[16], pass[16], msg[40] = ""; int ul, pl, field = 1, first = !has_user;
    cur_admin = 0;
    if (first) user[0] = 0; else str_ncpy(user, acct_user, 16);
    ul = strlen(user); pass[0] = 0; pl = 0;
    while (kbd_get() >= 0) {}
    const int dw = 236, dh = 118, dx = (SCR_W - dw) / 2, dy = (SCR_H - dh) / 2, fx = dx + 94, fw = 130;
    for (;;) {
        MouseEvt e; int k, submit = 0, guest = 0;
        while (mouse_next(&e))
            if (e.type == MOUSE_DOWN && e.btn == BTN_L) {
                if (ui_in(e.x, e.y, fx, dy + 40, fw, 12)) field = 1;
                else if (ui_in(e.x, e.y, fx, dy + 58, fw, 12)) field = 2;
                else if (first) { if (ui_in(e.x, e.y, dx + 88, dy + 82, 60, 16)) submit = 1; }
                else if (ui_in(e.x, e.y, dx + 50, dy + 82, 50, 16)) submit = 1;
                else if (ui_in(e.x, e.y, dx + 136, dy + 82, 50, 16)) guest = 1;
            }
        while ((k = kbd_get()) >= 0) {
            if (k == KEY_TAB) field = field == 1 ? 2 : 1;
            else if (k == KEY_ENTER) { if (field == 1) field = 2; else submit = 1; }
            else if (k == KEY_BS) { if (field == 1 && ul) user[--ul] = 0; else if (field == 2 && pl) pass[--pl] = 0; }
            else if (k >= 32 && k < 127) {
                if (field == 1 && ul < 15) { user[ul++] = k; user[ul] = 0; }
                else if (field == 2 && pl < 15) { pass[pl++] = k; pass[pl] = 0; }
            }
        }
        if (guest) { strcpy(cur_user, "guest"); return; }
        if (submit) {
            if (first) {                                   /* first login creates the account */
                if (!ul) { strcpy(msg, "Enter a user name."); field = 1; }
                else if (str_eq_nocase(user, "admin")) { strcpy(msg, "Name is reserved."); field = 1; }
                else { str_ncpy(acct_user, user, 16); str_ncpy(acct_pass, pass, 16); has_user = 1; settings_save(); strcpy(cur_user, acct_user); return; }
            } else if (str_eq_nocase(user, "admin") && strcmp(pass, admin_pass) == 0) { strcpy(cur_user, "admin"); cur_admin = 1; return; }
            else if (str_eq_nocase(user, acct_user) && strcmp(pass, acct_pass) == 0) { strcpy(cur_user, acct_user); return; }
            else { strcpy(msg, "Invalid name or password."); pass[0] = 0; pl = 0; field = 2; }
        }
        gfx_reset(); gfx_wallpaper();
        gfx_frame(dx, dy, dw, dh, C_BLACK); gfx_bevel(dx + 1, dy + 1, dw - 2, dh - 2, 1);
        gfx_rect(dx + 3, dy + 3, dw - 6, TITLE_H, theme_active[cfg.theme]);
        const char *tt = first ? "Create Account" : "CCORE Login";
        gfx_text(dx + 3 + (dw - 6 - (int)strlen(tt) * 8) / 2, dy + 4, tt, theme_text[cfg.theme]);
        gfx_text(dx + 10, dy + 20, "Welcome to CCORE 0.2", C_BLACK);
        gfx_text(dx + 10, dy + 42, "User name:", C_BLACK); gfx_text(dx + 10, dy + 60, "Password:", C_BLACK);
        gfx_sunken(fx, dy + 40, fw, 12, C_WHITE); gfx_sunken(fx, dy + 58, fw, 12, C_WHITE);
        gfx_text(fx + 3, dy + 42, user, C_BLACK);
        for (int i = 0; i < pl; i++) gfx_text(fx + 3 + i * 8, dy + 60, "*", C_BLACK);
        if ((sys_ticks / 50) & 1) gfx_vline(fx + 3 + (field == 1 ? ul : pl) * 8, (field == 1 ? dy + 42 : dy + 60), 8, C_BLACK);
        if (first) gfx_button(dx + 88, dy + 82, 60, 16, "Create", 0);
        else { gfx_button(dx + 50, dy + 82, 50, 16, "OK", 0); gfx_button(dx + 136, dy + 82, 50, 16, "Guest", 0); }
        gfx_text(dx + 10, dy + 72, msg, C_RED);
        gfx_text(dx + 10, dy + 102, first ? "New account will be created" : "Press Enter to log in", C_DGRAY);
        draw_mouse(); gfx_flip();
        sleep_ticks(2);
    }
}
#endif

void kernel_main(void) {
    gfx_reset();
    icons_arrange();
    mouse_init();
    __asm__ volatile("sti");
    vbe_init();
    post_screen();
    loading_screen();
    if (cfg.res && dwm_set_resolution(cfg.res)) cfg.res = 0;
#ifdef SETUP_BUILD
    setup_run();
#else
    for (;;) { login_screen(); desktop_run(); }
#endif
}

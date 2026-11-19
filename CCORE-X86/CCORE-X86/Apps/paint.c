/* CCORE v0.2 - paint.c : pixel paint with real canvas buffer, 16/256 colour palette */
#include "../System/DWM.h"

#define PW 176
#define PH 150
#define CX 66
#define CY 4
static struct { u8 canvas[PW * PH]; u8 color; int size, tool, pal256, lastx, lasty, drawing, mx, my; u8 pen; } p;
static u16 stk[8192];

static void dot(int x, int y, u8 c) {
    int h = p.size / 2;
    for (int j = 0; j < p.size; j++)
        for (int i = 0; i < p.size; i++) {
            int px = x - h + i, py = y - h + j;
            if (px >= 0 && px < PW && py >= 0 && py < PH) p.canvas[py * PW + px] = c;
        }
}
static void line(int x0, int y0, int x1, int y1, u8 c) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y0 - y1 : y1 - y0, sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx + dy;
    for (;;) {
        dot(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
static void flood(int sx, int sy, u8 nc) {
    if (sx < 0 || sy < 0 || sx >= PW || sy >= PH) return;
    u8 old = p.canvas[sy * PW + sx];
    if (old == nc) return;
    int sp = 0;
    p.canvas[sy * PW + sx] = nc; stk[sp++] = sy * PW + sx;
    while (sp) {
        int i = stk[--sp], x = i % PW, y = i / PW;
        int nb[4] = { x > 0 ? i - 1 : -1, x < PW - 1 ? i + 1 : -1, y > 0 ? i - PW : -1, y < PH - 1 ? i + PW : -1 };
        for (int k = 0; k < 4; k++)
            if (nb[k] >= 0 && p.canvas[nb[k]] == old && sp < 8192) { p.canvas[nb[k]] = nc; stk[sp++] = nb[k]; }
    }
}
static void draw(Window *w) {
    (void)w;
    gfx_sunken(3, 3, 50, 50, C_BLACK);
    if (!p.pal256) for (int i = 0; i < 16; i++) gfx_rect(4 + (i % 4) * 12, 4 + (i / 4) * 12, 12, 12, i);
    else for (int i = 0; i < 256; i++) gfx_rect(4 + (i % 16) * 3, 4 + (i / 16) * 3, 3, 3, i);
    gfx_frame(3, 56, 26, 12, C_BLACK); gfx_rect(4, 57, 24, 10, p.color);
    gfx_button(30, 56, 26, 12, p.pal256 ? "16" : "256", 0);
    gfx_button(4, 72, 27, 12, "Pen", p.tool == 0); gfx_button(33, 72, 27, 12, "Era", p.tool == 1);
    gfx_button(4, 86, 27, 12, "Fil", p.tool == 2); gfx_button(33, 86, 27, 12, "Clr", 0);
    gfx_text(4, 102, "Size", C_BLACK);
    static const char *sz[4] = { "1", "2", "4", "6" };
    for (int i = 0; i < 4; i++) gfx_button(4 + i * 15, 112, 13, 12, sz[i], p.size == (i == 0 ? 1 : i == 1 ? 2 : i == 2 ? 4 : 6));
    char b[16], *q = fmt_u(b, p.mx); *q++ = ','; fmt_u(q, p.my);
    gfx_text(4, 130, b, C_DGRAY);
    q = fmt_str(b, "#"); fmt_u(q, p.color); gfx_text(4, 140, b, C_DGRAY);
    gfx_frame(CX - 1, CY - 1, PW + 2, PH + 2, C_BLACK);
    gfx_blit(CX, CY, PW, PH, p.canvas);
}
static void mouse(Window *w, int t, int x, int y, int b) {
    (void)w;
    int cx = x - CX, cy = y - CY;
    if (cx >= 0 && cx < PW && cy >= 0 && cy < PH) { p.mx = cx; p.my = cy; }
    if (t == MOUSE_DOWN) {
        if (ui_in(x, y, 4, 4, 48, 48)) {
            p.color = p.pal256 ? ((y - 4) / 3) * 16 + (x - 4) / 3 : ((y - 4) / 12) * 4 + (x - 4) / 12;
            if (p.tool == 1) p.tool = 0;
        } else if (ui_in(x, y, 30, 56, 26, 12)) p.pal256 ^= 1;
        else if (ui_in(x, y, 4, 72, 27, 12)) p.tool = 0;
        else if (ui_in(x, y, 33, 72, 27, 12)) p.tool = 1;
        else if (ui_in(x, y, 4, 86, 27, 12)) p.tool = 2;
        else if (ui_in(x, y, 33, 86, 27, 12)) memset(p.canvas, C_WHITE, sizeof p.canvas);
        else if (ui_in(x, y, 4, 112, 58, 12)) { static const int s[4] = { 1, 2, 4, 6 }; int i = (x - 4) / 15; if (i < 4) p.size = s[i]; }
        else if (cx >= 0 && cx < PW && cy >= 0 && cy < PH) {
            if (p.tool == 2 && b == BTN_L) flood(cx, cy, p.color);
            else { p.pen = (b == BTN_R || p.tool == 1) ? C_WHITE : p.color; p.drawing = 1; p.lastx = cx; p.lasty = cy; dot(cx, cy, p.pen); }
        }
    } else if (t == MOUSE_MOVE && p.drawing) {
        line(p.lastx, p.lasty, cx, cy, p.pen); p.lastx = cx; p.lasty = cy;
    } else if (t == MOUSE_UP) p.drawing = 0;
}
void paint_launch(void) {
    Window *w = win_find(APP_PAINT);
    if (w) { win_focus(w); return; }
    w = win_create("Paint", APP_PAINT, 20, 6, CX + PW + 4, 158, app_mem_kb(sizeof p + sizeof stk));
    if (!w) return;
    memset(&p, 0, sizeof p); memset(p.canvas, C_WHITE, sizeof p.canvas); p.color = C_BLACK; p.size = 2;
    w->draw = draw; w->mouse = mouse;
}

/* CCORE v0.2 - settings.c : control panel (Graphics, Personalization, Mouse, Drivers, System) */
#include "../System/DWM.h"

enum { S_GRAPHICS, S_PERSONAL, S_MOUSE, S_DRIVERS, S_SYSTEM, S_COUNT };
static const char *const sec_name[S_COUNT] = { "Graphics", "Personalization", "Mouse", "Drivers", "System" };
#define PX 130                       /* content origin (client coords) */
static struct { int sec, field, ul, pl, al; char user[16], pass[16], apw[16], status[24]; } s;

static void save(void) { settings_save(); }
static void head(void) { gfx_text(PX, 4, sec_name[s.sec], C_BLUE); gfx_hline(PX, 14, 170, C_DGRAY); }
static void arrows(int y, const char *txt) {
    gfx_button(PX, y, 16, 12, "<", 0); gfx_button(PX + 18, y, 16, 12, ">", 0);
    gfx_text(PX + 40, y + 2, txt, C_BLACK);
}
static void stepper(int y, int v) {
    char b[8]; fmt_u(b, v);
    gfx_button(PX, y, 16, 12, "-", 0); gfx_button(PX + 18, y, 16, 12, "+", 0);
    gfx_text(PX + 40, y + 2, b, C_BLACK);
}
static void swatches(int y) {
    for (int i = 0; i < 16; i++) gfx_rect(PX + i * 10, y, 10, 10, i);
}
static void draw(Window *w) {
    (void)w;
    for (int i = 0; i < S_COUNT; i++) gfx_button(2, 4 + i * 22, 124, 18, sec_name[i], s.sec == i);
    gfx_vline(128, 2, 164, C_DGRAY);
    head();
    char b[32], *p;
    switch (s.sec) {
    case S_GRAPHICS:
        gfx_text(PX, 18, "Resolution", C_DGRAY);
        for (int i = 0; i < 3; i++) {
            int y = 30 + i * 14;
            gfx_text(PX, y, cur_res == i ? "(*)" : "( )", C_BLACK);
            gfx_text(PX + 28, y, res_names[i], (i && !vbe_ok) ? C_DGRAY : C_BLACK);
        }
        gfx_text(PX, 76, "Colors:", C_DGRAY); gfx_text(PX, 86, "256 (8 bpp)", C_BLACK);
        gfx_text(PX, 98, "Video memory:", C_DGRAY); p = fmt_str(b, "0x"); fmt_hex(p, gfx_fb_addr(), 8); gfx_text(PX, 108, b, C_BLACK);
        gfx_text(PX, 120, "Buffering:", C_DGRAY); p = fmt_str(b, "Double, "); p = fmt_u(p, SCR_W * SCR_H); fmt_str(p, " B");
        gfx_text(PX, 130, b, C_BLACK);
        gfx_text(PX, 142, "VGA palette:", C_DGRAY); swatches(152);
        break;
    case S_PERSONAL:
        gfx_text(PX, 20, "Desktop color", C_DGRAY); swatches(30);
        gfx_frame(PX + cfg.wall_color * 10 - 1, 29, 12, 12, C_WHITE); gfx_frame(PX + cfg.wall_color * 10, 30, 10, 10, C_BLACK);
        gfx_text(PX, 48, "Pattern", C_DGRAY); arrows(58, pattern_names[cfg.wall_pattern]);
        gfx_text(PX, 76, "Window theme", C_DGRAY); arrows(86, theme_names[cfg.theme]);
        gfx_rect(PX, 104, 100, 10, theme_active[cfg.theme]); gfx_text(PX + 2, 105, "Sample", theme_text[cfg.theme]);
        gfx_text(PX, 122, "Desktop icons", C_DGRAY); gfx_button(PX, 132, 116, 14, "Arrange icons", 0);
        gfx_text(PX, 150, "Drag: move icon", C_DGRAY); gfx_text(PX, 160, "Right-click: rename", C_DGRAY);
        break;
    case S_MOUSE:
        gfx_button(PX, 20, 96, 14, cfg.trails ? "Trails: ON" : "Trails: OFF", 0);
        gfx_text(PX, 40, "Trail length", C_DGRAY); stepper(50, cfg.trail_len);
        gfx_text(PX, 68, "Speed", C_DGRAY); stepper(78, cfg.mouse_speed);
        gfx_hline(PX, 98, 170, C_DGRAY);
        p = fmt_str(b, "X: "); p = fmt_u(p, mouse_x); p = fmt_str(p, " Y: "); fmt_u(p, mouse_y);
        gfx_text(PX, 106, b, C_BLACK);
        p = fmt_str(b, "L:"); p = fmt_str(p, (mouse_btn & 1) ? "down" : "up"); p = fmt_str(p, " R:"); fmt_str(p, (mouse_btn & 2) ? "down" : "up");
        gfx_text(PX, 118, b, C_BLACK);
        gfx_text(PX, 130, mouse_found ? "PS/2 mouse detected" : "PS/2 mouse missing", mouse_found ? C_BLACK : C_RED);
        break;
    case S_DRIVERS: {
        static const char *const nm[6] = { "Keyboard", "Mouse", "Timer", "ATA disk", "VGA", "RTC" };
        const char *st[6] = { "Active", mouse_found ? "Active" : "Missing", "100 Hz", ata_ok ? "Ready" : "None", "Mode 13h", "Active" };
        for (int i = 0; i < 6; i++) {
            gfx_text(PX, 20 + i * 11, nm[i], C_BLACK);
            gfx_text(PX + 80, 20 + i * 11, st[i], (i == 3 && !ata_ok) || (i == 1 && !mouse_found) ? C_RED : C_DGRAY);
        }
        gfx_text(PX, 94, "Disk model:", C_DGRAY); gfx_text_n(PX, 104, ata_ok ? ata_model : "Not detected", C_BLACK, 21);
        gfx_text(PX, 118, "Disk size:", C_DGRAY);
        if (ata_ok) { p = fmt_u(b, ata_sectors / 2048); fmt_str(p, " MB"); } else strcpy(b, "Not detected");
        gfx_text(PX, 128, b, C_BLACK);
        p = fmt_str(b, "Ticks: "); fmt_u(p, sys_ticks); gfx_text(PX, 144, b, C_BLACK);
        break; }
    case S_SYSTEM:
        gfx_text(PX, 20, "Account", C_DGRAY);
        gfx_text(PX, 32, "User:", C_BLACK); gfx_sunken(PX + 44, 30, 126, 12, s.field == 1 ? C_YELLOW : C_WHITE); gfx_text(PX + 47, 32, s.user, C_BLACK);
        gfx_text(PX, 48, "Pass:", C_BLACK); gfx_sunken(PX + 44, 46, 126, 12, s.field == 2 ? C_YELLOW : C_WHITE);
        for (int i = 0; i < s.pl; i++) gfx_text(PX + 47 + i * 8, 48, "*", C_BLACK);
        if (s.field && (sys_ticks / 50) & 1) gfx_vline(PX + 47 + (s.field == 1 ? s.ul : s.pl) * 8, s.field == 1 ? 32 : 48, 8, C_BLACK);
        gfx_text(PX, 64, "Admin:", C_BLACK);
        if (cur_admin) {
            gfx_sunken(PX + 44, 62, 126, 12, s.field == 3 ? C_YELLOW : C_WHITE);
            for (int i = 0; i < s.al; i++) gfx_text(PX + 47 + i * 8, 64, "*", C_BLACK);
            if (s.field == 3 && (sys_ticks / 50) & 1) gfx_vline(PX + 47 + s.al * 8, 64, 8, C_BLACK);
        } else gfx_text(PX + 44, 64, "(admin only)", C_DGRAY);
        gfx_button(PX, 80, 60, 14, "Apply", 0);
        gfx_text(PX, 98, s.status, C_BLUE);
        gfx_hline(PX, 110, 170, C_DGRAY);
        p = fmt_str(b, "RAM: "); p = fmt_u(p, ram_kb); fmt_str(p, " KB"); gfx_text(PX, 116, b, C_BLACK);
        p = fmt_str(b, "Uptime: "); fmt_hms(p, uptime_secs()); gfx_text(PX, 127, b, C_BLACK);
        gfx_button(PX, 146, 84, 14, "Test BSOD", 0);
        break;
    }
}
static void mouse(Window *w, int t, int x, int y, int b) {
    (void)w; (void)b;
    if (t != MOUSE_DOWN) return;
    for (int i = 0; i < S_COUNT; i++) if (ui_in(x, y, 2, 4 + i * 22, 124, 18)) { s.sec = i; s.field = 0; return; }
    int bx = x - PX;
    switch (s.sec) {
    case S_GRAPHICS:
        for (int i = 0; i < 3; i++)
            if (ui_in(x, y, PX, 28 + i * 14, 170, 12)) {
                if (dwm_set_resolution(i)) dwm_message("Mode not supported here"); else save();
            }
        break;
    case S_PERSONAL:
        if (ui_in(x, y, PX, 30, 160, 10)) { cfg.wall_color = bx / 10; wallpaper_rebuild(); save(); }
        else if (ui_in(x, y, PX, 58, 16, 12)) { cfg.wall_pattern = (cfg.wall_pattern + NUM_PATTERNS - 1) % NUM_PATTERNS; wallpaper_rebuild(); save(); }
        else if (ui_in(x, y, PX + 18, 58, 16, 12)) { cfg.wall_pattern = (cfg.wall_pattern + 1) % NUM_PATTERNS; wallpaper_rebuild(); save(); }
        else if (ui_in(x, y, PX, 86, 16, 12)) { cfg.theme = (cfg.theme + NUM_THEMES - 1) % NUM_THEMES; save(); }
        else if (ui_in(x, y, PX + 18, 86, 16, 12)) { cfg.theme = (cfg.theme + 1) % NUM_THEMES; save(); }
        else if (ui_in(x, y, PX, 132, 116, 14)) { icons_arrange(); save(); }
        break;
    case S_MOUSE:
        if (ui_in(x, y, PX, 20, 96, 14)) { cfg.trails ^= 1; save(); }
        else if (ui_in(x, y, PX, 50, 16, 12)) { if (cfg.trail_len > 1) cfg.trail_len--; save(); }
        else if (ui_in(x, y, PX + 18, 50, 16, 12)) { if (cfg.trail_len < 15) cfg.trail_len++; save(); }
        else if (ui_in(x, y, PX, 78, 16, 12)) { if (cfg.mouse_speed > 1) cfg.mouse_speed--; save(); }
        else if (ui_in(x, y, PX + 18, 78, 16, 12)) { if (cfg.mouse_speed < 8) cfg.mouse_speed++; save(); }
        break;
    case S_SYSTEM:
        if (ui_in(x, y, PX + 44, 30, 126, 12)) s.field = 1;
        else if (ui_in(x, y, PX + 44, 46, 126, 12)) s.field = 2;
        else if (cur_admin && ui_in(x, y, PX + 44, 62, 126, 12)) s.field = 3;
        else if (ui_in(x, y, PX, 80, 60, 14)) {
            if (!s.ul) { str_ncpy(s.status, "User name is empty", 24); return; }
            if (str_eq_nocase(s.user, "admin")) { str_ncpy(s.status, "Name is reserved", 24); return; }
            str_ncpy(acct_user, s.user, 16); str_ncpy(acct_pass, s.pass, 16);
            if (!cur_admin) strcpy(cur_user, acct_user);
            has_user = 1;
            if (cur_admin && s.al) str_ncpy(admin_pass, s.apw, 16);
            save(); str_ncpy(s.status, fs_ok ? "Account saved to disk" : "Account set (no disk)", 24);
        } else if (ui_in(x, y, PX, 146, 84, 14)) bsod_manual();
        break;
    }
}
static void key(Window *w, int k) {
    (void)w;
    if (s.sec != S_SYSTEM || !s.field) return;
    if (k == KEY_TAB) s.field = s.field == 1 ? 2 : (s.field == 2 && cur_admin) ? 3 : 1;
    else if (k == KEY_ENTER || k == KEY_ESC) s.field = 0;
    else if (k == KEY_BS) { if (s.field == 1 && s.ul) s.user[--s.ul] = 0; else if (s.field == 2 && s.pl) s.pass[--s.pl] = 0; else if (s.field == 3 && s.al) s.apw[--s.al] = 0; }
    else if (k >= 32 && k < 127) {
        if (s.field == 1 && s.ul < 15) { s.user[s.ul++] = k; s.user[s.ul] = 0; }
        else if (s.field == 2 && s.pl < 15) { s.pass[s.pl++] = k; s.pass[s.pl] = 0; }
        else if (s.field == 3 && s.al < 15) { s.apw[s.al++] = k; s.apw[s.al] = 0; }
    }
}
void settings_launch(void) {
    Window *w = win_find(APP_SETTINGS);
    if (w) { win_focus(w); return; }
    w = win_create("Control Panel", APP_SETTINGS, 6, 2, 304, 168, app_mem_kb(sizeof s));
    if (!w) return;
    memset(&s, 0, sizeof s); str_ncpy(s.user, acct_user, 16); s.ul = strlen(s.user);
    w->draw = draw; w->mouse = mouse; w->key = key;
}

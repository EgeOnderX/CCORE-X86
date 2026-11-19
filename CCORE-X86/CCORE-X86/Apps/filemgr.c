/* CCORE v0.2 - filemgr.c : file manager over the real ATA-backed CFS directory */
#include "../System/DWM.h"

#define VIS 10
static struct { int sel, top, naming, namelen; char name[NAME_MAX_LEN + 1]; u32 clk; int lastclk; } f;
static int list[MAX_FILES], nlist;
static void rebuild(void) {
    nlist = 0;
    for (int i = 0; i < MAX_FILES; i++) if (fs_dir[i].used) list[nlist++] = i;
    if (f.sel >= nlist) f.sel = nlist - 1;
    if (f.top > nlist - VIS) f.top = nlist - VIS;
    if (f.top < 0) f.top = 0;
}
static int is_txt(const char *s) {
    int n = strlen(s);
    if (n <= 4) return 0;
    return str_eq_nocase(s + n - 4, ".TXT") || str_eq_nocase(s + n - 4, ".BAT") || str_eq_nocase(s + n - 4, ".SYS") || str_eq_nocase(s + n - 4, ".INI");
}
static void deny(int r) { dwm_message(r == -5 ? "Access denied: admin only" : "Disk error"); }
static void copy_sel(void) {
    if (f.sel < 0) return;
    int src = list[f.sel], r = -2;
    static const char *const pre[5] = { "COPY_", "COP2_", "COP3_", "COP4_", "COP5_" };
    for (int i = 0; i < 5 && r == -2; i++) {
        char nm[NAME_MAX_LEN + 1]; strcpy(nm, pre[i]);
        str_ncpy(nm + 5, fs_dir[src].name, NAME_MAX_LEN + 1 - 5);
        r = fs_copy(src, nm);
    }
    if (r == -1) dwm_message("Directory full"); else if (r < 0) deny(r);
    else { rebuild(); for (int i = 0; i < nlist; i++) if (list[i] == r) f.sel = i; }
}
static void open_sel(void) {
    if (f.sel < 0) return;
    const char *nm = fs_dir[list[f.sel]].name;
    if (is_txt(nm)) notepad_open_file(nm); else dwm_message("Only text files can be opened");
}
static void create_named(void) {
    if (!f.namelen) { f.naming = 0; return; }
    char nm[NAME_MAX_LEN + 1]; strcpy(nm, f.name);
    int dot = 0; for (int i = 0; nm[i]; i++) if (nm[i] == '.') dot = 1;
    if (!dot) { if (f.namelen + 4 > NAME_MAX_LEN) { dwm_message("Name too long"); return; } strcpy(nm + f.namelen, ".TXT"); }
    int r = fs_create(nm);
    if (r == -2) { dwm_message("File already exists"); return; }
    if (r == -1) { dwm_message("Directory full"); return; }
    if (r < 0) { dwm_message("Disk error"); return; }
    f.naming = 0; rebuild();
    for (int i = 0; i < nlist; i++) if (list[i] == r) f.sel = i;
}
static void draw(Window *w) {
    (void)w; rebuild();
    gfx_button(4, 4, 34, 13, "New", 0); gfx_button(40, 4, 34, 13, "Del", 0); gfx_button(76, 4, 40, 13, "Open", 0); gfx_button(118, 4, 54, 13, "Rescan", 0); gfx_button(174, 4, 36, 13, "Copy", 0);
    gfx_text(6, 21, "Name", C_BLUE); gfx_text(164, 21, "Size", C_BLUE);
    gfx_sunken(4, 30, 206, VIS * 9 + 4, C_WHITE);
    for (int r = 0; r < VIS && f.top + r < nlist; r++) {
        int i = f.top + r, y = 32 + r * 9; const DirEntry *d = &fs_dir[list[i]];
        if (i == f.sel) gfx_rect(5, y, 204, 9, C_BLUE);
        u8 col = i == f.sel ? C_WHITE : C_BLACK;
        gfx_text(7, y + 1, d->name, col);
        char b[12]; fmt_u(b, d->size); gfx_text(164, y + 1, b, col);
        if (fs_is_sys(list[i])) gfx_text(200, y + 1, "S", i == f.sel ? C_WHITE : C_RED);
    }
    gfx_vscroll(212, 30, 10, VIS * 9 + 4, f.top, nlist, VIS);
    u32 tot = 0; for (int i = 0; i < nlist; i++) tot += fs_dir[list[i]].size;
    char b[48], *p = fmt_u(b, nlist); p = fmt_str(p, " file(s), "); p = fmt_u(p, tot); fmt_str(p, " bytes");
    gfx_text(4, 128, fs_ok ? b : "No disk / file system", fs_ok ? C_BLACK : C_RED);
    if (f.naming) {
        gfx_text(4, 141, "New:", C_BLACK); gfx_sunken(40, 138, 172, 14, C_WHITE);
        gfx_text(43, 141, f.name, C_BLACK);
        if ((sys_ticks / 50) & 1) gfx_vline(43 + f.namelen * 8, 140, 9, C_BLACK);
    } else { p = fmt_u(b, MAX_FILES - nlist); p = fmt_str(p, " free slots"); gfx_text(4, 141, b, C_DGRAY); }
}
static void mouse(Window *w, int t, int x, int y, int b) {
    (void)w; (void)b;
    if (t != MOUSE_DOWN) return;
    if (ui_in(x, y, 4, 4, 34, 13)) { if (fs_ok) { f.naming = 1; f.namelen = 0; f.name[0] = 0; } else dwm_message("No disk detected"); }
    else if (ui_in(x, y, 40, 4, 34, 13)) { if (f.sel >= 0) { int r = fs_delete(list[f.sel]); if (r) deny(r); else rebuild(); } }
    else if (ui_in(x, y, 174, 4, 36, 13)) copy_sel();
    else if (ui_in(x, y, 76, 4, 40, 13)) open_sel();
    else if (ui_in(x, y, 118, 4, 54, 13)) { if (fs_rescan()) dwm_message("Disk error"); rebuild(); }
    else if (ui_in(x, y, 5, 32, 204, VIS * 9)) {
        int i = f.top + (y - 32) / 9;
        if (i < nlist) {
            if (i == f.lastclk && sys_ticks - f.clk < 50) { f.sel = i; open_sel(); f.lastclk = -1; }
            else { f.sel = i; f.lastclk = i; f.clk = sys_ticks; }
        }
    } else if (ui_in(x, y, 212, 30, 10, VIS * 9 + 4)) { f.top += (y < 30 + (VIS * 9 + 4) / 2) ? -3 : 3; rebuild(); }
}
static void key(Window *w, int k) {
    (void)w;
    if (f.naming) {
        if (k == KEY_ESC) f.naming = 0;
        else if (k == KEY_ENTER) create_named();
        else if (k == KEY_BS) { if (f.namelen) f.name[--f.namelen] = 0; }
        else if (k > 32 && k < 127 && f.namelen < NAME_MAX_LEN) { f.name[f.namelen++] = (k >= 'a' && k <= 'z') ? k - 32 : k; f.name[f.namelen] = 0; }
        return;
    }
    if (k == KEY_UP && f.sel > 0) f.sel--;
    else if (k == KEY_DOWN && f.sel < nlist - 1) f.sel++;
    else if (k == KEY_ENTER) open_sel();
    else if (k == KEY_DEL) { if (f.sel >= 0) { int r = fs_delete(list[f.sel]); if (r) deny(r); } }
    if (f.sel < f.top) f.top = f.sel;
    if (f.sel >= f.top + VIS) f.top = f.sel - VIS + 1;
}
void filemgr_launch(void) {
    Window *w = win_find(APP_FILEMGR);
    if (w) { win_focus(w); return; }
    w = win_create("File Manager", APP_FILEMGR, 40, 10, 226, 156, app_mem_kb(sizeof f + sizeof list));
    if (!w) return;
    memset(&f, 0, sizeof f); f.sel = -1; f.lastclk = -1;
    w->draw = draw; w->mouse = mouse; w->key = key;
}

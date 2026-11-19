/* CCORE v0.2 - notepad.c : multi-line text editor with ATA-backed Save / Load */
#include "../System/DWM.h"

#define NP_MAX 4000
#define COLS 28
#define ROWS 11
static struct { char text[NP_MAX + 1]; int len, cur, top, namelen, editing_name, dirty; char fname[NAME_MAX_LEN + 1]; char status[32]; } n;

static void np_pos(int idx, int *row, int *col) {
    int r = 0, c = 0;
    for (int i = 0; i < idx; i++) {
        if (n.text[i] == '\n') { r++; c = 0; }
        else if (++c >= COLS) { r++; c = 0; }
    }
    *row = r; *col = c;
}
static int np_index(int row, int col) {
    int r = 0, c = 0, best = -1;
    for (int i = 0; i <= n.len; i++) {
        if (r == row) { if (c <= col) best = i; else break; }
        else if (r > row) break;
        if (i < n.len) {
            if (n.text[i] == '\n') { r++; c = 0; }
            else if (++c >= COLS) { r++; c = 0; }
        }
    }
    return best < 0 ? n.len : best;
}
static void set_status(const char *s) { str_ncpy(n.status, s, 32); }
static void do_new(void) { n.len = n.cur = n.top = n.dirty = 0; n.text[0] = 0; strcpy(n.fname, "UNTITLED.TXT"); n.namelen = strlen(n.fname); set_status("New document"); }
static void do_save(void) {
    if (!n.namelen) { set_status("No file name"); return; }
    int i = fs_find(n.fname);
    if (i < 0) i = fs_create(n.fname);
    if (i == -1) { set_status("Dir full"); return; }
    if (i < 0) { set_status("Bad name/disk"); return; }
    int wr = fs_write(i, n.text, n.len);
    if (wr == -5) { set_status("Access denied"); return; }
    if (wr) { set_status("Write failed"); return; }
    n.dirty = 0;
    char b[32], *p = fmt_str(b, "Saved "); p = fmt_u(p, n.len); fmt_str(p, " B"); set_status(b);
}
static void do_load(void) {
    int i = fs_find(n.fname);
    if (i < 0) { set_status("Not found"); return; }
    int r = fs_read(i, n.text, NP_MAX);
    if (r < 0) { set_status("Read failed"); return; }
    int j = 0;
    for (int k = 0; k < r; k++) { char ch = n.text[k]; if (ch == '\r') continue; if (ch == '\t') ch = ' '; n.text[j++] = ch; }
    n.len = j; n.text[j] = 0; n.cur = n.top = n.dirty = 0;
    char b[32], *p = fmt_str(b, "Loaded "); p = fmt_u(p, n.len); fmt_str(p, " B"); set_status(b);
}
static void insert(char ch) {
    if (n.len >= NP_MAX) { set_status("Buffer full"); return; }
    memmove(&n.text[n.cur + 1], &n.text[n.cur], n.len - n.cur + 1);
    n.text[n.cur++] = ch; n.len++; n.dirty = 1;
}
static void draw(Window *w) {
    (void)w;
    gfx_button(4, 4, 34, 14, "New", 0); gfx_button(40, 4, 34, 14, "Load", 0); gfx_button(76, 4, 34, 14, "Save", 0);
    gfx_sunken(114, 4, 126, 14, n.editing_name ? C_YELLOW : C_WHITE);
    int shown = n.namelen > 14 ? 14 : n.namelen;
    gfx_text_n(117, 7, n.fname + (n.namelen - shown), C_BLACK, shown);
    if (n.editing_name && (sys_ticks / 50) & 1) gfx_vline(117 + shown * 8, 6, 9, C_BLACK);
    gfx_sunken(4, 22, COLS * 8 + 4, ROWS * 8 + 4, C_WHITE);
    int r = 0, c = 0, crow, ccol;
    np_pos(n.cur, &crow, &ccol);
    if (crow < n.top) n.top = crow;
    if (crow >= n.top + ROWS) n.top = crow - ROWS + 1;
    for (int i = 0; i < n.len; i++) {
        if (n.text[i] == '\n') { r++; c = 0; continue; }
        if (r >= n.top && r < n.top + ROWS) gfx_char(6 + c * 8, 24 + (r - n.top) * 8, n.text[i], C_BLACK);
        if (++c >= COLS) { r++; c = 0; }
    }
    if (!n.editing_name && ((sys_ticks / 50) & 1)) gfx_rect(6 + ccol * 8, 24 + (crow - n.top) * 8, 2, 8, C_BLACK);
    int lr, lc; np_pos(n.len, &lr, &lc);
    gfx_vscroll(COLS * 8 + 10, 22, 10, ROWS * 8 + 4, n.top, lr + 1, ROWS);
    char b[40], *p = fmt_str(b, "Ln "); p = fmt_u(p, crow + 1); p = fmt_str(p, " Col "); p = fmt_u(p, ccol + 1); if (n.dirty) fmt_str(p, " *");
    gfx_text(4, 118, b, C_BLACK);
    gfx_text_n(134, 118, n.status, C_BLUE, 13);
}
static void mouse(Window *w, int t, int x, int y, int b) {
    (void)w; (void)b;
    if (t != MOUSE_DOWN) return;
    if (ui_in(x, y, 4, 4, 34, 14)) do_new();
    else if (ui_in(x, y, 40, 4, 34, 14)) do_load();
    else if (ui_in(x, y, 76, 4, 34, 14)) do_save();
    else if (ui_in(x, y, 114, 4, 126, 14)) n.editing_name = 1;
    else if (ui_in(x, y, 4, 22, COLS * 8 + 4, ROWS * 8 + 4)) {
        n.editing_name = 0;
        int col = (x - 6) / 8, row = n.top + (y - 24) / 8;
        n.cur = np_index(row < 0 ? 0 : row, col < 0 ? 0 : col);
    }
}
static void key(Window *w, int k) {
    (void)w;
    if (n.editing_name) {
        if (k == KEY_ENTER || k == KEY_ESC || k == KEY_TAB) n.editing_name = 0;
        else if (k == KEY_BS) { if (n.namelen) n.fname[--n.namelen] = 0; }
        else if (k > 32 && k < 127 && n.namelen < NAME_MAX_LEN) { n.fname[n.namelen++] = (k >= 'a' && k <= 'z') ? k - 32 : k; n.fname[n.namelen] = 0; }
        return;
    }
    int row, col;
    switch (k) {
    case KEY_LEFT: if (n.cur > 0) n.cur--; break;
    case KEY_RIGHT: if (n.cur < n.len) n.cur++; break;
    case KEY_UP: np_pos(n.cur, &row, &col); if (row > 0) n.cur = np_index(row - 1, col); break;
    case KEY_DOWN: np_pos(n.cur, &row, &col); n.cur = np_index(row + 1, col); break;
    case KEY_HOME: np_pos(n.cur, &row, &col); n.cur = np_index(row, 0); break;
    case KEY_END: np_pos(n.cur, &row, &col); n.cur = np_index(row, 9999); break;
    case KEY_PGUP: np_pos(n.cur, &row, &col); n.cur = np_index(row > ROWS ? row - ROWS : 0, col); break;
    case KEY_PGDN: np_pos(n.cur, &row, &col); n.cur = np_index(row + ROWS, col); break;
    case KEY_BS: if (n.cur > 0) { memmove(&n.text[n.cur - 1], &n.text[n.cur], n.len - n.cur + 1); n.cur--; n.len--; n.dirty = 1; } break;
    case KEY_DEL: if (n.cur < n.len) { memmove(&n.text[n.cur], &n.text[n.cur + 1], n.len - n.cur); n.len--; n.dirty = 1; } break;
    case KEY_ENTER: insert('\n'); break;
    case KEY_TAB: insert(' '); insert(' '); break;
    default: if (k >= 32 && k < 127) insert(k);
    }
}
static void tick(Window *w) {
    char b[32], *p = fmt_str(b, "Notepad - "); fmt_str(p, n.fname);
    str_ncpy(w->title, b, 32);
}
void notepad_launch(void) {
    Window *w = win_find(APP_NOTEPAD);
    if (w) { win_focus(w); return; }
    w = win_create("Notepad", APP_NOTEPAD, 30, 20, COLS * 8 + 20, 130, app_mem_kb(sizeof n));
    if (!w) return;
    memset(&n, 0, sizeof n); do_new();
    w->draw = draw; w->mouse = mouse; w->key = key; w->tick = tick;
}
void notepad_open_file(const char *name) {
    notepad_launch();
    str_ncpy(n.fname, name, NAME_MAX_LEN + 1); n.namelen = strlen(n.fname); n.editing_name = 0;
    do_load();
}

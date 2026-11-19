/* CCORE v0.2 - calc.c : fixed-point (3 decimals) calculator */
#include "../System/DWM.h"

typedef struct { int x, y, w, h; char label[3]; char key; } CBtn;
static const CBtn btns[] = {
    {4,22,24,18,"C",'C'}, {30,22,24,18,"<",'B'}, {56,22,24,18,"*",'*'}, {82,22,24,18,"/",'/'},
    {4,42,24,18,"7",'7'}, {30,42,24,18,"8",'8'}, {56,42,24,18,"9",'9'}, {82,42,24,18,"-",'-'},
    {4,62,24,18,"4",'4'}, {30,62,24,18,"5",'5'}, {56,62,24,18,"6",'6'}, {82,62,24,18,"+",'+'},
    {4,82,24,18,"1",'1'}, {30,82,24,18,"2",'2'}, {56,82,24,18,"3",'3'}, {82,82,24,38,"=",'='},
    {4,102,50,18,"0",'0'}, {56,102,24,18,".",'.'} };
#define NB ((int)(sizeof btns / sizeof btns[0]))

static struct { s32 acc, cur; char op, entry[14]; int typing, err, pressed; } c;

static s32 parse(const char *e) {
    s32 ip = 0, fr = 0; int fd = 0, dot = 0, neg = 0;
    for (; *e; e++) {
        if (*e == '-') neg = 1;
        else if (*e == '.') dot = 1;
        else if (!dot) ip = ip * 10 + (*e - '0');
        else if (fd < 3) { fr = fr * 10 + (*e - '0'); fd++; }
    }
    for (; fd < 3; fd++) fr *= 10;
    s32 v = ip * 1000 + fr;
    return neg ? -v : v;
}
static void fx_str(s32 v, char *o) {
    if (v < 0) { *o++ = '-'; v = -v; }
    o = fmt_u(o, (u32)v / 1000);
    u32 f = (u32)v % 1000;
    if (f) { *o++ = '.'; *o++ = '0' + f / 100; *o++ = '0' + (f / 10) % 10; *o++ = '0' + f % 10;
        while (o[-1] == '0') { o--; }
        *o = 0; }
}
/* 64/32 -> 32 divide with overflow flag, using hardware divl (no libgcc needed) */
static int div64(u32 hi, u32 lo, u32 d, u32 *q) {
    u32 qh = hi / d, r = hi % d, ql;
    __asm__("divl %4" : "=a"(ql), "=d"(r) : "a"(lo), "d"(r), "r"(d));
    *q = ql;
    return qh == 0 && ql <= 0x7FFFFFFF;
}
static s32 compute(s32 a, char op, s32 b) {
    s32 r = 0;
    switch (op) {
    case '+': if (__builtin_add_overflow(a, b, &r)) c.err = 1; break;
    case '-': if (__builtin_sub_overflow(a, b, &r)) c.err = 1; break;
    case '*': case '/': {
        int neg = (a < 0) ^ (b < 0);
        u32 ua = a < 0 ? -(u32)a : (u32)a, ub = b < 0 ? -(u32)b : (u32)b, q = 0;
        if (op == '*') { u64 p = (u64)ua * ub; if (!div64((u32)(p >> 32), (u32)p, 1000, &q)) c.err = 1; }
        else if (ub == 0) c.err = 1;
        else { u64 p = (u64)ua * 1000; if (!div64((u32)(p >> 32), (u32)p, ub, &q)) c.err = 1; }
        r = neg ? -(s32)q : (s32)q;
    } break;
    }
    return r;
}
static void apply_pending(void) {
    if (c.op && c.typing) { c.cur = compute(c.acc, c.op, c.cur); c.acc = c.cur; }
    else if (!c.op) c.acc = c.cur;
}
static void press(char k) {
    if (c.err && k != 'C') return;
    if ((k >= '0' && k <= '9') || k == '.') {
        if (!c.typing) { c.entry[0] = 0; c.typing = 1; }
        int n = strlen(c.entry), dot = 0, dec = 0, ip = 0;
        for (int i = 0; i < n; i++) { if (c.entry[i] == '.') dot = 1; else if (dot) dec++; else ip++; }
        if (k == '.') { if (dot) return; if (!n) c.entry[n++] = '0'; c.entry[n++] = '.'; }
        else {
            if (dot && dec >= 3) return;
            if (!dot && ip >= 7) return;
            if (n == 1 && c.entry[0] == '0') n = 0;
            c.entry[n++] = k;
        }
        c.entry[n] = 0; c.cur = parse(c.entry);
    } else if (k == 'C') { memset(&c, 0, sizeof c); }
    else if (k == 'B') { int n = strlen(c.entry); if (c.typing && n) { c.entry[n - 1] = 0; c.cur = parse(c.entry); } }
    else if (k == '=') { apply_pending(); c.op = 0; c.typing = 0; }
    else { apply_pending(); c.op = k; c.typing = 0; }
}
static void draw(Window *w) {
    (void)w;
    gfx_sunken(4, 4, 102, 14, C_WHITE);
    char buf[16];
    if (c.err) strcpy(buf, "Error");
    else if (c.typing) strcpy(buf, c.entry[0] ? c.entry : "0");
    else fx_str(c.cur, buf);
    gfx_text(4 + 100 - strlen(buf) * 8, 7, buf, c.err ? C_RED : C_BLACK);
    if (c.op) gfx_char(7, 7, c.op, C_BLUE);
    for (int i = 0; i < NB; i++) gfx_button(btns[i].x, btns[i].y, btns[i].w, btns[i].h, btns[i].label, c.pressed == i + 1);
}
static void mouse(Window *w, int t, int x, int y, int b) {
    (void)w; (void)b;
    if (t == MOUSE_DOWN) for (int i = 0; i < NB; i++) if (ui_in(x, y, btns[i].x, btns[i].y, btns[i].w, btns[i].h)) { c.pressed = i + 1; press(btns[i].key); }
    if (t == MOUSE_UP) c.pressed = 0;
}
static void key(Window *w, int k) {
    (void)w;
    if ((k >= '0' && k <= '9') || k == '.' || k == '+' || k == '-' || k == '*' || k == '/' || k == '=') press(k);
    else if (k == KEY_ENTER) press('=');
    else if (k == KEY_BS) press('B');
    else if (k == 'c' || k == 'C' || k == KEY_ESC) press('C');
}
void calc_launch(void) {
    Window *w = win_find(APP_CALC);
    if (w) { win_focus(w); return; }
    w = win_create("Calculator", APP_CALC, 170, 12, 110, 124, app_mem_kb(sizeof c));
    if (!w) return;
    memset(&c, 0, sizeof c);
    w->draw = draw; w->mouse = mouse; w->key = key;
}

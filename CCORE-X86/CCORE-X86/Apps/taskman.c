/* CCORE v0.2 - taskman.c : live task table, PIT uptime, End Task */
#include "../System/DWM.h"

#define VIS 11
static struct { int sel_pid, top; } t;
static int pids[MAX_WINDOWS + 1];
static int gather(void) {            /* PID 0 = DWM kernel task, then windows sorted by PID */
    int n = 0; pids[n++] = 0;
    for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) pids[n++] = wins[i].pid;
    for (int i = 2; i < n; i++) { int v = pids[i], j = i - 1; while (j >= 1 && pids[j] > v) { pids[j + 1] = pids[j]; j--; } pids[j + 1] = v; }
    return n;
}
static void end_task(int pid) {
    if (pid < 0) return;
    if (pid == 0) {
        if (!cur_admin) dwm_message("Access denied: admin only");
        else bsod_stop("CRITICAL_PROCESS_DIED", "DWM.SYS was terminated (End Task).");
        return;
    }
    Window *k = win_by_pid(pid);
    t.sel_pid = -1;
    if (k) win_close(k);
}
static u32 kernel_kb(void) { return ((u32)__bss_end - 0x10000 + 1023) / 1024; }
static void draw(Window *w) {
    (void)w;
    int n = gather();
    char b[48], *p = fmt_str(b, "Uptime "); fmt_hms(p, uptime_secs()); p = b + strlen(b);
    p = fmt_str(p, "  Tasks "); fmt_u(p, n);
    gfx_text(4, 4, b, C_BLACK);
    gfx_text(5, 16, "PID", C_BLUE); gfx_text(36, 16, "Task", C_BLUE); gfx_text(148, 16, "Mem", C_BLUE); gfx_text(188, 16, "State", C_BLUE);
    gfx_sunken(2, 25, 236, VIS * 9 + 4, C_WHITE);
    u32 used = kernel_kb();
    for (int r = 0; r < VIS && t.top + r < n; r++) {
        int pid = pids[t.top + r], y = 27 + r * 9; Window *wn = win_by_pid(pid);
        if (pid == t.sel_pid) gfx_rect(3, y, 234, 9, C_BLUE);
        u8 c = pid == t.sel_pid ? C_WHITE : C_BLACK;
        u32 mem = pid ? wn->mem_kb : kernel_kb();
        const char *st = !pid ? "System" : wn->minimized ? "Min" : wn == win_top() ? "Focus" : "Run";
        p = fmt_u(b, pid); gfx_text(5, y + 1, b, c);
        gfx_text_n(36, y + 1, pid ? wn->title : "DWM.SYS", c, 14);
        p = fmt_u(b, mem); fmt_str(p, "K"); gfx_text(148, y + 1, b, c);
        gfx_text(188, y + 1, st, c);
    }
    for (int i = 1; i < n; i++) used += win_by_pid(pids[i])->mem_kb;
    p = fmt_str(b, "Mem "); p = fmt_u(p, used); p = fmt_str(p, "K / "); p = fmt_u(p, ram_kb / 1024); fmt_str(p, "M");
    gfx_text(78, 136, b, C_BLACK);
    gfx_button(4, 132, 68, 16, "End Task", 0);
}
static void mouse(Window *w, int ty, int x, int y, int b) {
    (void)w; (void)b;
    if (ty != MOUSE_DOWN) return;
    int n = gather();
    if (ui_in(x, y, 3, 27, 234, VIS * 9)) { int i = t.top + (y - 27) / 9; if (i < n) t.sel_pid = pids[i]; }
    else if (ui_in(x, y, 4, 132, 68, 16)) {
        end_task(t.sel_pid);
    }
}
static void key(Window *w, int k) {
    (void)w;
    int n = gather(), i = 0;
    for (int j = 0; j < n; j++) if (pids[j] == t.sel_pid) i = j;
    if (k == KEY_UP && i > 0) i--; else if (k == KEY_DOWN && i < n - 1) i++;
    else if (k == KEY_DEL) { end_task(t.sel_pid); return; }
    t.sel_pid = pids[i];
    if (i < t.top) t.top = i;
    if (i >= t.top + VIS) t.top = i - VIS + 1;
}
void taskman_launch(void) {
    Window *w = win_find(APP_TASKMAN);
    if (w) { win_focus(w); return; }
    w = win_create("Task Manager", APP_TASKMAN, 36, 8, 240, 152, app_mem_kb(sizeof t + sizeof pids));
    if (!w) return;
    memset(&t, 0, sizeof t); t.sel_pid = -1;
    w->draw = draw; w->mouse = mouse; w->key = key;
}

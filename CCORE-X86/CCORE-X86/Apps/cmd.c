/* CCORE v0.2 - cmd.c : MS-DOS style command prompt (optionally elevated with the admin password) */
#include "../System/DWM.h"

#define COLS 38
#define ROWS 19
#define LINES 100
static struct {
    char t[LINES][COLS + 1]; u8 wrap[LINES];
    int cur, total, cx, view;
    char in[64]; int il;
    int mode, elev; u8 fg, bg;
    char hist[8][64]; int hn, hp;
    Window *win;
} c;
static u8 fbuf[FILE_MAXB + 1];

/* ---------- console ---------- */
static void newline(void) {
    c.cur = (c.cur + 1) % LINES; memset(c.t[c.cur], 0, COLS + 1); c.wrap[c.cur] = 0; c.cx = 0;
    if (c.total < LINES) c.total++;
}
static void putc_(int ch) {
    if (ch == '\r') return;
    if (ch == '\n') { newline(); return; }
    if (ch == '\t') ch = ' ';
    if (ch < 32 || ch > 126) ch = '?';
    if (c.cx >= COLS) { newline(); c.wrap[c.cur] = 1; }
    c.t[c.cur][c.cx++] = ch; c.t[c.cur][c.cx] = 0;
}
static void out(const char *s) { while (*s) putc_(*s++); }
static void outln(const char *s) { out(s); putc_('\n'); }
static void outu(u32 v) { char b[12]; fmt_u(b, v); out(b); }
static void cls(void) { memset(c.t, 0, sizeof c.t); memset(c.wrap, 0, sizeof c.wrap); c.cur = 0; c.total = 1; c.cx = 0; c.view = 0; }
static int admin(void) { return c.elev || cur_admin; }
static void prompt(void) {
    if (c.mode == 1) out("Password: ");
    else out(admin() ? "C:\\#>" : "C:\\>");
}
static void set_title(void) {
    if (c.win) str_ncpy(c.win->title, admin() ? "Administrator: Command Prompt" : "Command Prompt", 32);
}

/* ---------- helpers ---------- */
static const char *const img_names[] = { "", "CALC.EXE", "CVER.EXE", "FILEMGR.EXE", "NOTEPAD.EXE", "PAINT.EXE", "SETTINGS.EXE", "TASKMAN.EXE", "MSGBOX.EXE", "CMD.EXE" };
static const char *img(const Window *w) { return w->app >= 1 && w->app <= APP_CMD ? img_names[w->app] : "APP.EXE"; }
static int split(char *s, char **argv, int max) {
    int n = 0;
    while (*s && n < max) { while (*s == ' ') s++; if (!*s) break; argv[n++] = s; while (*s && *s != ' ') s++; if (*s) *s++ = 0; }
    return n;
}
static int str_up(int ch) { return (ch >= 'a' && ch <= 'z') ? ch - 32 : ch; }
static int is(const char *a, const char *b) { return str_eq_nocase(a, b); }
static void pad(const char *s, int w) { int n = strlen(s); out(s); for (; n < w; n++) putc_(' '); }
static void date_time(int date) {
    char b[24], *p;
    if (date) { int y, m, d; rtc_date(&y, &m, &d); p = fmt_u(b, y); *p++ = '-'; p = fmt_2d(p, m); *p++ = '-'; fmt_2d(p, d); }
    else { int h, m, s; rtc_read(&h, &m, &s); p = fmt_2d(b, h); *p++ = ':'; p = fmt_2d(p, m); *p++ = ':'; fmt_2d(p, s); }
    outln(b);
}
static int need_fs(void) { if (!fs_ok) { outln("No disk / file system."); return 0; } return 1; }
static void fs_err(int r, const char *name) {
    if (r == -5) outln("Access is denied (admin only).");
    else if (r == -2) outln("A file with that name exists.");
    else if (r == -1 && name) { out("File not found - "); outln(name); }
    else if (r == -4) outln("Invalid file name.");
    else outln("Disk or directory error.");
}
static void start_app(const char *n, const char *arg) {
    if (is(n, "calc")) calc_launch();
    else if (is(n, "notepad") || is(n, "edit")) { if (arg) notepad_open_file(arg); else notepad_launch(); }
    else if (is(n, "paint")) paint_launch();
    else if (is(n, "filemgr") || is(n, "files")) filemgr_launch();
    else if (is(n, "settings") || is(n, "control")) settings_launch();
    else if (is(n, "taskman") || is(n, "taskmgr")) taskman_launch();
    else if (is(n, "cver") || is(n, "about")) cver_launch();
    else if (is(n, "cmd")) cmd_launch(0);
    else { out("Cannot find "); outln(n); }
}

/* ---------- commands ---------- */
static void cmd_help(void) {
    outln("HELP     CLS      VER      EXIT");
    outln("DIR      TYPE f   COPY a b DEL f");
    outln("REN a b  ECHO t   ECHO t > f");
    outln("DATE     TIME     MEM      VOL");
    outln("CHKDSK   WHOAMI   COLOR xy  TITLE t");
    outln("TASKLIST TASKKILL /PID n | /IM name");
    outln("START app (calc notepad paint files");
    outln("   settings taskman about cmd)");
    outln("EDIT f   ATTRIB [+S|-S] f");
    outln("RUNAS    SHUTDOWN RESTART");
    outln("Admin only: ATTRIB +S/-S, BSOD,");
    outln("system files, TASKKILL of DWM.SYS");
}
static void cmd_dir(void) {
    if (!need_fs()) return;
    outln(" Volume in drive C is CCORE"); outln("");
    int n = 0; u32 tot = 0;
    for (int i = 0; i < MAX_FILES; i++) if (fs_dir[i].used) {
        char b[12]; pad(fs_dir[i].name, 20); fmt_u(b, fs_dir[i].size);
        for (int k = strlen(b); k < 7; k++) putc_(' ');
        out(b); out(fs_is_sys(i) ? " S" : "  "); putc_('\n');
        n++; tot += fs_dir[i].size;
    }
    outu(n); out(" File(s) "); outu(tot); outln(" bytes");
    outu(MAX_FILES - n); outln(" free slot(s)");
}
static void cmd_type(const char *name) {
    int i = fs_find(name);
    if (i < 0) { fs_err(-1, name); return; }
    int n = fs_read(i, fbuf, FILE_MAXB);
    if (n < 0) { fs_err(-3, NULL); return; }
    for (int k = 0; k < n; k++) putc_(fbuf[k]);
    if (n && fbuf[n - 1] != '\n') putc_('\n');
}
static void cmd_copy(const char *a, const char *b) {
    int i = fs_find(a);
    if (i < 0) { fs_err(-1, a); return; }
    int r = fs_copy(i, b);
    if (r < 0) fs_err(r, a); else outln("        1 file(s) copied.");
}
static void cmd_echo(const char *rest) {
    char txt[64]; str_ncpy(txt, rest, 64);
    char *gt = NULL;
    for (char *q = txt; *q; q++) if (*q == '>') { gt = q; break; }
    if (!gt) { outln(txt); return; }
    *gt++ = 0;
    int l = strlen(txt); while (l > 0 && txt[l - 1] == ' ') txt[--l] = 0;
    while (*gt == ' ') gt++;
    if (!*gt) { outln("Missing file name."); return; }
    if (!need_fs()) return;
    int i = fs_find(gt);
    if (i < 0) i = fs_create(gt);
    if (i < 0) { fs_err(i, gt); return; }
    txt[l++] = '\n'; txt[l] = 0;
    int r = fs_write(i, txt, l);
    if (r) fs_err(r, gt);
}
static void cmd_mem(void) {
    u32 sys = ((u32)__bss_end - 0x10000 + 1023) / 1024, tasks = 0;
    for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) tasks += wins[i].mem_kb;
    out("Total  : "); outu(ram_kb); outln(" KB");
    out("System : "); outu(sys); outln(" KB");
    out("Tasks  : "); outu(tasks); outln(" KB");
    out("Free   : "); outu(ram_kb - sys - tasks); outln(" KB");
}
static void cmd_chkdsk(void) {
    if (!need_fs()) return;
    int n = 0; u32 tot = 0;
    for (int i = 0; i < MAX_FILES; i++) if (fs_dir[i].used) { n++; tot += fs_dir[i].size; }
    out("Disk   : "); outu(ata_sectors / 2048); outln(" MB");
    out("Files  : "); outu(n); out(" of "); outu(MAX_FILES); putc_('\n');
    out("Data   : "); outu(tot); outln(" bytes");
}
static void cmd_tasklist(void) {
    outln("PID  Image Name            Mem");
    outln("---  ------------------  ------");
    char b[12];
    fmt_u(b, 0); pad(b, 5); pad("DWM.SYS", 20); outu(((u32)__bss_end - 0x10000 + 1023) / 1024); outln(" K");
    for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) {
        fmt_u(b, wins[i].pid); pad(b, 5); pad(img(&wins[i]), 20); outu(wins[i].mem_kb); outln(" K");
    }
}
static void kill_win(Window *w, int pid) {
    out("SUCCESS: terminated PID "); outu(pid); putc_('\n');
    win_close(w);
}
static void cmd_taskkill(int argc, char **argv) {
    if (argc < 3) { outln("Usage: TASKKILL /PID n | /IM name [/F]"); return; }
    if (is(argv[1], "/pid")) {
        int pid = 0; for (const char *q = argv[2]; *q >= '0' && *q <= '9'; q++) pid = pid * 10 + (*q - '0');
        if (pid == 0) {
            if (!admin()) { outln("ERROR: Access is denied."); return; }
            bsod_stop("CRITICAL_PROCESS_DIED", "DWM.SYS was terminated (TASKKILL).");
            return;
        }
        Window *w = win_by_pid(pid);
        if (!w) { out("ERROR: process "); outu(pid); outln(" not found."); return; }
        kill_win(w, pid);
    } else if (is(argv[1], "/im")) {
        if (is(argv[2], "dwm.sys") || is(argv[2], "dwm")) {
            if (!admin()) { outln("ERROR: Access is denied."); return; }
            bsod_stop("CRITICAL_PROCESS_DIED", "DWM.SYS was terminated (TASKKILL).");
            return;
        }
        for (int i = 0; i < MAX_WINDOWS; i++) if (wins[i].used) {
            char nm[16]; str_ncpy(nm, img(&wins[i]), 16);
            int l = strlen(argv[2]);
            int match = is(argv[2], nm);
            if (!match && l + 4 == (int)strlen(nm) && str_eq_nocase(nm + l, ".EXE")) { match = 1; for (int k = 0; k < l; k++) if (str_up(argv[2][k]) != nm[k]) match = 0; }
            if (match) { kill_win(&wins[i], wins[i].pid); return; }
        }
        out("ERROR: process \""); out(argv[2]); outln("\" not found.");
    } else outln("Usage: TASKKILL /PID n | /IM name [/F]");
}
static void cmd_attrib(int argc, char **argv) {
    if (!need_fs()) return;
    if (argc == 1) {
        for (int i = 0; i < MAX_FILES; i++) if (fs_dir[i].used) { out(fs_is_sys(i) ? "S   " : "    "); outln(fs_dir[i].name); }
        return;
    }
    if (argc == 2) { int i = fs_find(argv[1]); if (i < 0) fs_err(-1, argv[1]); else { out(fs_is_sys(i) ? "S   " : "    "); outln(fs_dir[i].name); } return; }
    int on = is(argv[1], "+s");
    if (!on && !is(argv[1], "-s")) { outln("Usage: ATTRIB [+S|-S] file"); return; }
    int i = fs_find(argv[2]);
    if (i < 0) { fs_err(-1, argv[2]); return; }
    int r = fs_set_sys(i, on);
    if (r) fs_err(r, argv[2]);
}

static void exec_line(const char *line) {
    char raw[64], work[64], *argv[6];
    str_ncpy(raw, line, 64);
    char *p = raw; while (*p == ' ') p++;
    if (*p == '@') p++;
    if (!*p || is(p, "rem") || (p[0] == 'r' && p[1] == 'e' && p[2] == 'm' && p[3] == ' ')) return;
    str_ncpy(work, p, 64);
    int argc = split(work, argv, 6);
    if (!argc) return;
    const char *rest = p; while (*rest && *rest != ' ') rest++; while (*rest == ' ') rest++;
    int sv = cur_admin; if (c.elev) cur_admin = 1;
    const char *cm = argv[0];
    if (is(cm, "help") || is(cm, "?")) cmd_help();
    else if (is(cm, "cls")) cls();
    else if (is(cm, "ver")) outln("CCORE Operating System [Version " OS_VERSION "]");
    else if (is(cm, "exit")) { cur_admin = sv; win_close(c.win); return; }
    else if (is(cm, "dir")) cmd_dir();
    else if (is(cm, "type")) { if (argc < 2) outln("Usage: TYPE file"); else if (need_fs()) cmd_type(argv[1]); }
    else if (is(cm, "copy")) { if (argc < 3) outln("Usage: COPY source target"); else if (need_fs()) cmd_copy(argv[1], argv[2]); }
    else if (is(cm, "del") || is(cm, "erase")) {
        if (argc < 2) outln("Usage: DEL file");
        else if (need_fs()) { int i = fs_find(argv[1]); if (i < 0) fs_err(-1, argv[1]); else { int r = fs_delete(i); if (r) fs_err(r, argv[1]); } }
    } else if (is(cm, "ren") || is(cm, "rename")) {
        if (argc < 3) outln("Usage: REN old new");
        else if (need_fs()) { int i = fs_find(argv[1]); if (i < 0) fs_err(-1, argv[1]); else { int r = fs_rename(i, argv[2]); if (r) fs_err(r, argv[1]); } }
    } else if (is(cm, "echo")) { if (!*rest) outln("ECHO is on."); else cmd_echo(rest); }
    else if (is(cm, "date")) { out("Current date: "); date_time(1); }
    else if (is(cm, "time")) { out("Current time: "); date_time(0); }
    else if (is(cm, "mem")) cmd_mem();
    else if (is(cm, "vol")) { outln("Volume in drive C is CCORE"); out("Disk: "); outln(ata_ok ? ata_model : "not detected"); }
    else if (is(cm, "chkdsk")) cmd_chkdsk();
    else if (is(cm, "whoami")) { out(cur_user[0] ? cur_user : "user"); outln(admin() ? " (administrator)" : ""); }
    else if (is(cm, "tasklist")) cmd_tasklist();
    else if (is(cm, "taskkill")) cmd_taskkill(argc, argv);
    else if (is(cm, "start")) { if (argc < 2) outln("Usage: START app"); else start_app(argv[1], argc > 2 ? argv[2] : NULL); }
    else if (is(cm, "edit")) start_app("notepad", argc > 1 ? argv[1] : NULL);
    else if (is(cm, "attrib")) cmd_attrib(argc, argv);
    else if (is(cm, "color")) {
        if (argc < 2 || strlen(argv[1]) != 2) outln("Usage: COLOR bf (hex digits)");
        else {
            int d[2];
            for (int k = 0; k < 2; k++) { int ch = argv[1][k]; d[k] = (ch >= '0' && ch <= '9') ? ch - '0' : (ch >= 'a' && ch <= 'f') ? ch - 'a' + 10 : (ch >= 'A' && ch <= 'F') ? ch - 'A' + 10 : -1; }
            if (d[0] < 0 || d[1] < 0 || d[0] == d[1]) outln("Invalid color."); else { c.bg = d[0]; c.fg = d[1]; }
        }
    } else if (is(cm, "title")) { if (*rest && c.win) str_ncpy(c.win->title, rest, 32); }
    else if (is(cm, "runas") || is(cm, "su")) {
        if (admin()) outln("Already running as administrator.");
        else { c.mode = 1; }
    } else if (is(cm, "shutdown")) sys_shutdown();
    else if (is(cm, "restart") || is(cm, "reboot")) sys_reboot();
    else if (is(cm, "bsod")) { if (!admin()) outln("Access is denied (admin only)."); else bsod_manual(); }
    else { out("'"); out(cm); outln("' is not recognized as a command."); }
    cur_admin = sv;
}
static void run_autoexec(void) {
    int i = fs_ok ? fs_find("AUTOEXEC.BAT") : -1;
    if (i < 0) return;
    int n = fs_read(i, fbuf, FILE_MAXB);
    char line[64]; int l = 0;
    for (int k = 0; k < n; k++) {
        if (fbuf[k] == '\n' || l >= 63) { line[l] = 0; l = 0; exec_line(line); }
        else if (fbuf[k] != '\r') line[l++] = fbuf[k];
    }
    if (l) { line[l] = 0; exec_line(line); }
}

/* ---------- window ---------- */
static void draw(Window *w) {
    (void)w;
    gfx_rect(0, 0, COLS * 8 + 8, ROWS * 8 + 8, c.bg);
    int maxv = c.total > ROWS ? c.total - ROWS : 0;
    if (c.view > maxv) c.view = maxv;
    for (int r = 0; r < ROWS; r++) {
        int back = ROWS - 1 - r + c.view;
        if (back >= c.total) continue;
        gfx_text(4, 4 + r * 8, c.t[(c.cur - back + LINES * 2) % LINES], c.fg);
    }
    if (!c.view && ((sys_ticks / 40) & 1)) gfx_rect(4 + c.cx * 8, 4 + (ROWS - 1) * 8 + 6, 7, 2, c.fg);
}
static void submit(void) {
    putc_('\n');
    if (c.mode == 1) {
        c.mode = 0;
        if (strcmp(c.in, admin_pass) == 0) { c.elev = 1; outln("Administrator privileges granted."); set_title(); }
        else outln("Logon failure: wrong password.");
    } else if (c.il) {
        if (c.hn < 8) c.hn++;
        for (int i = c.hn - 1; i > 0; i--) strcpy(c.hist[i], c.hist[i - 1]);
        strcpy(c.hist[0], c.in);
        exec_line(c.in);
    }
    c.il = 0; c.in[0] = 0; c.hp = -1;
    if (c.win && c.win->used) prompt();
}
static void erase_char(void) {
    if (!c.il) return;
    c.in[--c.il] = 0;
    if (c.cx == 0 && c.wrap[c.cur]) { c.cur = (c.cur + LINES - 1) % LINES; c.cx = COLS; c.total--; }
    c.cx--; c.t[c.cur][c.cx] = 0;
}
static void key(Window *w, int k) {
    (void)w;
    if (k == KEY_PGUP) { c.view += 5; return; }
    if (k == KEY_PGDN) { c.view -= 5; if (c.view < 0) c.view = 0; return; }
    c.view = 0;
    if (k == KEY_ENTER) submit();
    else if (k == KEY_BS) erase_char();
    else if (k == KEY_ESC) { while (c.il) erase_char(); }
    else if ((k == KEY_UP || k == KEY_DOWN) && c.mode == 0) {
        int np = c.hp + (k == KEY_UP ? 1 : -1);
        if (np < -1 || np >= c.hn) return;
        while (c.il) erase_char();
        c.hp = np;
        if (np >= 0) { for (const char *q = c.hist[np]; *q; q++) { c.in[c.il++] = *q; putc_(*q); } c.in[c.il] = 0; }
    } else if (k >= 32 && k < 127 && c.il < 62) { c.in[c.il++] = k; c.in[c.il] = 0; putc_(c.mode == 1 ? '*' : k); }
}
void cmd_launch(int adm) {
    Window *w = win_find(APP_CMD);
    int fresh = 0;
    if (!w) {
        w = win_create("Command Prompt", APP_CMD, 2, 2, COLS * 8 + 8, ROWS * 8 + 8, app_mem_kb(sizeof c + sizeof fbuf));
        if (!w) return;
        memset(&c, 0, sizeof c);
        c.fg = C_LGRAY; c.bg = C_BLACK; c.hp = -1; c.win = w; c.total = 1;
        w->draw = draw; w->key = key; w->bg = C_BLACK;
        fresh = 1;
    }
    win_focus(w);
    if (fresh) {
        outln("CCORE Operating System [Version " OS_VERSION "]"); outln("");
        set_title(); run_autoexec();
        prompt();
    }
    if (adm && !admin() && c.mode == 0) {
        while (c.il) erase_char();
        putc_('\n'); outln("Elevation required. Enter the");
        outln("administrator password.");
        c.mode = 1; prompt();
    }
}

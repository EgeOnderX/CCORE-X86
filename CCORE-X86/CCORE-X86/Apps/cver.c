/* CCORE v0.2 - cver.c : system version & metadata dialog (live readouts) */
#include "../System/DWM.h"

static void draw(Window *w) {
    char b[48], *p;
    gfx_rect(8, 8, 28, 28, C_BLUE); gfx_frame(8, 8, 28, 28, C_BLACK);
    gfx_text_big(11, 15, "C", C_WHITE, 2); gfx_text_big(21, 21, "C", C_YELLOW, 1);
    gfx_text(46, 12, "CCORE Operating System", C_BLACK);
    gfx_text(46, 24, "Version " OS_VERSION, C_BLACK);
    gfx_hline(8, 42, w->cw - 16, C_DGRAY); gfx_hline(8, 43, w->cw - 16, C_WHITE);
    p = fmt_str(b, "RAM:   "); p = fmt_u(p, ram_kb); p = fmt_str(p, " KB ("); p = fmt_u(p, ram_kb / 1024); fmt_str(p, " MB)");
    gfx_text(8, 50, b, C_BLACK);
    u32 cr0 = read_cr0();
    p = fmt_str(b, "Mode:  "); fmt_str(p, (cr0 & 1) ? ((cr0 & 0x80000000u) ? "32-bit PM, paging" : "32-bit protected") : "real mode");
    gfx_text(8, 60, b, C_BLACK);
    int y = 70;
    if (cpu_ok) {
        gfx_text(8, y, "CPU:", C_BLACK); y += 10;
        gfx_text_n(8, y, cpu_name, C_BLACK, 26); y += 10;
        if (strlen(cpu_name) > 26) { gfx_text_n(8, y, cpu_name + 26, C_BLACK, 26); y += 10; }
    } else { gfx_text(8, y, "CPU:   Not detected", C_BLACK); y += 10; }
    p = fmt_str(b, "User:  "); fmt_str(p, cur_user); gfx_text(8, y, b, C_BLACK); y += 10;
    if (ata_ok) { p = fmt_str(b, "Disk:  "); p = fmt_u(p, ata_sectors / 2048); fmt_str(p, " MB ATA PIO"); }
    else strcpy(b, "Disk:  Not detected");
    gfx_text(8, y, b, C_BLACK); y += 10;
    p = fmt_str(b, "Video: "); fmt_str(p, res_names[cur_res]); gfx_text(8, y, b, C_BLACK); y += 10;
    int yy, mo, dd; rtc_date(&yy, &mo, &dd);
    p = fmt_str(b, "Date:  "); p = fmt_u(p, yy); *p++ = '-'; p = fmt_2d(p, mo); *p++ = '-'; fmt_2d(p, dd);
    gfx_text(8, y, b, C_BLACK); y += 10;
    p = fmt_str(b, "Up:    "); fmt_hms(p, uptime_secs()); gfx_text(8, y, b, C_BLACK);
    gfx_button((w->cw - 50) / 2, 150, 50, 16, "OK", 0);
}
static void mouse(Window *w, int t, int x, int y, int b) { (void)b; if (t == MOUSE_DOWN && ui_in(x, y, (w->cw - 50) / 2, 150, 50, 16)) win_close(w); }
static void key(Window *w, int k) { if (k == KEY_ENTER || k == KEY_ESC) win_close(w); }
void cver_launch(void) {
    Window *w = win_find(APP_CVER);
    if (w) { win_focus(w); return; }
    w = win_create("About CCORE", APP_CVER, 40, 2, 232, 168, app_mem_kb(0));
    if (!w) return;
    w->draw = draw; w->mouse = mouse; w->key = key;
}

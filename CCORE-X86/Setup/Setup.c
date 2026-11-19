/* CCORE Setup : first-run wizard (account, date/time, personalization) that installs CCORE to the disk.
 * Built into the installer kernel with -DSETUP_BUILD. Boots like normal CCORE, then this wizard opens. */
#include "../CCORE-X86/System/DWM.h"

#define PAYLOAD_BOOT_LBA 999
#define PAYLOAD_KERNEL_LBA 1000
#define KERNEL_SECTORS 256
enum { P_WELCOME, P_DISPLAY, P_ADMIN, P_ACCOUNT, P_DATETIME, P_PERSONAL, P_READY, P_INSTALL, P_DONE };
#define DW 280
#define DH 180
#define DX ((SCR_W - DW) / 2)
#define DY ((SCR_H - DH) / 2)

static int page, field, ul, pl, al, ey, emo, ed, eh, emi, pct;
static char user[16], pass[16], apass[16], msg[32];
static u8 iobuf[16 * 512];

static int days_in(int y, int m) {
    static const int d[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return (m == 2 && (y % 4 == 0)) ? 29 : d[m - 1];
}
static u8 bcd_if(int v) { outb(0x70, 0x0B); return (inb(0x71) & 4) ? (u8)v : (u8)(((v / 10) << 4) | (v % 10)); }
static void cmos_w(u8 r, u8 v) { outb(0x70, r); outb(0x71, v); }
static void set_rtc(void) {
    outb(0x70, 0x0B); u8 b = inb(0x71);
    cmos_w(0x0B, b | 0x80 | 0x02);                      /* SET + 24h */
    cmos_w(0x09, bcd_if(ey % 100)); cmos_w(0x08, bcd_if(emo)); cmos_w(0x07, bcd_if(ed));
    cmos_w(0x04, bcd_if(eh)); cmos_w(0x02, bcd_if(emi)); cmos_w(0x00, 0);
    cmos_w(0x0B, (b | 0x02) & ~0x80);
}
static void progress_draw(void) {
    gfx_reset(); gfx_wallpaper();
    gfx_frame(DX, DY, DW, DH, C_BLACK); gfx_bevel(DX + 1, DY + 1, DW - 2, DH - 2, 1);
    gfx_rect(DX + 3, DY + 3, DW - 6, TITLE_H, theme_active[cfg.theme]);
    gfx_text(DX + 3 + (DW - 6 - 88) / 2, DY + 4, "CCORE Setup", theme_text[cfg.theme]);
    gfx_text(DX + 10, DY + 40, "Installing CCORE...", C_BLACK);
    gfx_sunken(DX + 10, DY + 60, 260, 14, C_WHITE);
    for (int i = 0; i < pct * 252 / 100 / 8; i++) gfx_rect(DX + 14 + i * 8, DY + 63, 7, 8, C_BLUE);
    gfx_flip();
}
static int install(void) {
    pct = 0; progress_draw();
    if (!ata_ok) { strcpy(msg, "No disk detected"); return -1; }
    if (ata_read(PAYLOAD_BOOT_LBA, 1, iobuf) || iobuf[510] != 0x55 || iobuf[511] != 0xAA) { strcpy(msg, "Install files missing"); return -1; }
    for (int s = 0; s < KERNEL_SECTORS; s += 16) {                 /* copy payload kernel over the installer kernel */
        if (ata_read(PAYLOAD_KERNEL_LBA + s, 16, iobuf) || ata_write(1 + s, 16, iobuf)) { strcpy(msg, "Disk error"); return -1; }
        pct = (s + 16) * 80 / KERNEL_SECTORS; progress_draw();
    }
    if (ata_read(PAYLOAD_BOOT_LBA, 1, iobuf) || ata_write(0, 1, iobuf)) { strcpy(msg, "Disk error"); return -1; }
    pct = 90; progress_draw();
    if (fs_format(apass, user, pass)) { strcpy(msg, "Disk error"); return -1; }
    set_rtc();
    pct = 100; progress_draw(); sleep_ticks(40);
    return 0;
}
static void arrows_draw(int y, const char *txt) {
    gfx_button(DX + 110, DY + y, 16, 12, "<", 0); gfx_button(DX + 128, DY + y, 16, 12, ">", 0);
    gfx_text(DX + 150, DY + y + 2, txt, C_BLACK);
}
static const char *const title[] = { "Welcome", "Display", "Administrator", "User Account", "Date and Time", "Personalization", "Ready to Install", "Installing", "Complete" };
static void draw(void) {
    gfx_reset(); gfx_wallpaper();
    gfx_frame(DX, DY, DW, DH, C_BLACK); gfx_bevel(DX + 1, DY + 1, DW - 2, DH - 2, 1);
    gfx_rect(DX + 3, DY + 3, DW - 6, TITLE_H, theme_active[cfg.theme]);
    gfx_text(DX + 3 + (DW - 6 - 88) / 2, DY + 4, "CCORE Setup", theme_text[cfg.theme]);
    gfx_text(DX + 10, DY + 18, title[page], C_BLUE); gfx_hline(DX + 10, DY + 28, DW - 20, C_DGRAY);
    char b[16];
    switch (page) {
    case P_WELCOME:
        gfx_text(DX + 10, DY + 40, "Welcome to CCORE Setup.", C_BLACK);
        gfx_text(DX + 10, DY + 60, "This wizard will:", C_BLACK);
        gfx_text(DX + 18, DY + 72, "- choose the screen resolution", C_BLACK);
        gfx_text(DX + 18, DY + 83, "- create the administrator", C_BLACK);
        gfx_text(DX + 18, DY + 94, "- create your user account", C_BLACK);
        gfx_text(DX + 18, DY + 105, "- set the date and time", C_BLACK);
        gfx_text(DX + 18, DY + 116, "- personalize the desktop", C_BLACK);
        gfx_text(DX + 18, DY + 127, "- install CCORE to the disk", C_BLACK);
        break;
    case P_DISPLAY:
        gfx_text(DX + 10, DY + 40, "Choose the screen resolution:", C_BLACK);
        for (int i = 0; i < 3; i++) {
            gfx_text(DX + 18, DY + 62 + i * 16, cur_res == i ? "(*)" : "( )", C_BLACK);
            gfx_text(DX + 52, DY + 62 + i * 16, res_names[i], (i && !vbe_ok) ? C_DGRAY : C_BLACK);
        }
        gfx_text(DX + 10, DY + 112, vbe_ok ? "Applied immediately." : "Only VGA is available here.", C_DGRAY);
        break;
    case P_ADMIN:
        gfx_text(DX + 10, DY + 40, "Create the administrator:", C_BLACK);
        gfx_text(DX + 10, DY + 62, "User name:", C_BLACK); gfx_text(DX + 100, DY + 62, "admin", C_BLUE);
        gfx_text(DX + 10, DY + 82, "Password:", C_BLACK);
        gfx_sunken(DX + 100, DY + 80, 130, 12, C_YELLOW);
        for (int i = 0; i < al; i++) gfx_text(DX + 103 + i * 8, DY + 82, "*", C_BLACK);
        if ((sys_ticks / 50) & 1) gfx_vline(DX + 103 + al * 8, DY + 82, 8, C_BLACK);
        gfx_text(DX + 10, DY + 100, "Admin can edit system files", C_DGRAY);
        gfx_text(DX + 10, DY + 111, "and end the DWM task.", C_DGRAY);
        break;
    case P_ACCOUNT:
        gfx_text(DX + 10, DY + 40, "Create your user account:", C_BLACK);
        gfx_text(DX + 10, DY + 62, "User name:", C_BLACK); gfx_text(DX + 10, DY + 82, "Password:", C_BLACK);
        gfx_sunken(DX + 100, DY + 60, 130, 12, field == 1 ? C_YELLOW : C_WHITE); gfx_sunken(DX + 100, DY + 80, 130, 12, field == 2 ? C_YELLOW : C_WHITE);
        gfx_text(DX + 103, DY + 62, user, C_BLACK);
        for (int i = 0; i < pl; i++) gfx_text(DX + 103 + i * 8, DY + 82, "*", C_BLACK);
        if ((sys_ticks / 50) & 1) gfx_vline(DX + 103 + (field == 1 ? ul : pl) * 8, DY + (field == 1 ? 62 : 82), 8, C_BLACK);
        gfx_text(DX + 10, DY + 104, "The password may be empty.", C_DGRAY);
        break;
    case P_DATETIME: {
        static const char *const lb[5] = { "Year", "Month", "Day", "Hour", "Minute" };
        int v[5] = { ey, emo, ed, eh, emi };
        for (int i = 0; i < 5; i++) {
            int y = 40 + i * 18;
            gfx_text(DX + 10, DY + y + 2, lb[i], C_BLACK);
            if (i == 0) fmt_u(b, v[i]); else fmt_2d(b, v[i]);
            gfx_sunken(DX + 90, DY + y, 40, 12, C_WHITE); gfx_text(DX + 93, DY + y + 2, b, C_BLACK);
            gfx_button(DX + 136, DY + y, 16, 12, "-", 0); gfx_button(DX + 154, DY + y, 16, 12, "+", 0);
        }
        break; }
    case P_PERSONAL:
        gfx_text(DX + 10, DY + 40, "Desktop color:", C_BLACK);
        for (int i = 0; i < 16; i++) gfx_rect(DX + 10 + i * 15, DY + 52, 14, 12, i);
        gfx_frame(DX + 9 + cfg.wall_color * 15, DY + 51, 16, 14, C_WHITE);
        gfx_text(DX + 10, DY + 80, "Pattern:", C_BLACK); arrows_draw(78, pattern_names[cfg.wall_pattern]);
        gfx_text(DX + 10, DY + 102, "Window theme:", C_BLACK); arrows_draw(100, theme_names[cfg.theme]);
        break;
    case P_READY:
        gfx_text(DX + 10, DY + 40, "Setup is ready to install.", C_BLACK);
        gfx_text(DX + 10, DY + 50, "Admin:", C_DGRAY); gfx_text(DX + 66, DY + 50, "admin", C_BLACK);
        gfx_text(DX + 10, DY + 61, "User:", C_DGRAY); gfx_text(DX + 66, DY + 61, user, C_BLACK);
        fmt_u(b, ey); gfx_text(DX + 10, DY + 72, "Date:", C_DGRAY); gfx_text(DX + 66, DY + 72, b, C_BLACK);
        fmt_2d(b, emo); gfx_text(DX + 106, DY + 72, b, C_BLACK); fmt_2d(b, ed); gfx_text(DX + 130, DY + 72, b, C_BLACK);
        fmt_2d(b, eh); gfx_text(DX + 162, DY + 72, b, C_BLACK); fmt_2d(b, emi); gfx_text(DX + 186, DY + 72, b, C_BLACK);
        gfx_text(DX + 10, DY + 92, "Click Install to write CCORE", C_BLACK);
        gfx_text(DX + 10, DY + 104, "to the disk.", C_BLACK);
        break;
    case P_DONE:
        gfx_text(DX + 10, DY + 40, "Setup is complete.", C_BLACK);
        gfx_text(DX + 10, DY + 58, "CCORE will now restart.", C_BLACK);
        break;
    }
    gfx_text(DX + 10, DY + 124, msg, C_RED);
    if (page > P_WELCOME && page < P_INSTALL) gfx_button(DX + 10, DY + 150, 70, 16, "Back", 0);
    if (page < P_READY) gfx_button(DX + DW - 80, DY + 150, 70, 16, "Next", 0);
    else if (page == P_READY) gfx_button(DX + DW - 80, DY + 150, 70, 16, "Install", 0);
    else if (page == P_DONE) gfx_button(DX + DW - 80, DY + 150, 70, 16, "Restart", 0);
    draw_mouse(); gfx_flip();
}
static void next(void) {
    msg[0] = 0;
    if (page == P_ADMIN && !al) { strcpy(msg, "Enter an admin password."); return; }
    if (page == P_ACCOUNT && !ul) { strcpy(msg, "Enter a user name."); field = 1; return; }
    if (page == P_ACCOUNT && str_eq_nocase(user, "admin")) { strcpy(msg, "Name is reserved."); field = 1; return; }
    if (page == P_READY) {
        page = P_INSTALL;
        if (install()) { page = P_READY; return; }
        page = P_DONE; return;
    }
    if (page == P_DONE) sys_reboot();
    page++;
}
static void adj(int which, int d) {
    switch (which) {
    case 0: ey += d; if (ey < 2000) ey = 2000; if (ey > 2099) ey = 2099; break;
    case 1: emo += d; if (emo < 1) emo = 12; if (emo > 12) emo = 1; break;
    case 2: ed += d; if (ed < 1) ed = days_in(ey, emo); if (ed > days_in(ey, emo)) ed = 1; break;
    case 3: eh += d; if (eh < 0) eh = 23; if (eh > 23) eh = 0; break;
    case 4: emi += d; if (emi < 0) emi = 59; if (emi > 59) emi = 0; break;
    }
    if (ed > days_in(ey, emo)) ed = days_in(ey, emo);
}
void setup_run(void) {
    int s;
    rtc_date(&ey, &emo, &ed); rtc_read(&eh, &emi, &s);
    if (ey < 2000) ey = 2026;
    if (emo < 1 || emo > 12) emo = 1;
    if (ed < 1 || ed > 31) ed = 1;
    field = 1; page = P_WELCOME; wallpaper_rebuild();
    while (kbd_get() >= 0) {}
    for (;;) {
        MouseEvt e; int k;
        while (mouse_next(&e)) {
            if (e.type != MOUSE_DOWN || e.btn != BTN_L || page == P_INSTALL) continue;
            if (page > P_WELCOME && page < P_INSTALL && ui_in(e.x, e.y, DX + 10, DY + 150, 70, 16)) { page--; msg[0] = 0; }
            else if (ui_in(e.x, e.y, DX + DW - 80, DY + 150, 70, 16)) next();
            else if (page == P_DISPLAY) {
                for (int i = 0; i < 3; i++)
                    if (ui_in(e.x, e.y, DX + 18, DY + 60 + i * 16, 200, 12)) { if (dwm_set_resolution(i)) strcpy(msg, "Mode not supported"); else msg[0] = 0; }
            } else if (page == P_ACCOUNT) {
                if (ui_in(e.x, e.y, DX + 100, DY + 60, 130, 12)) field = 1;
                else if (ui_in(e.x, e.y, DX + 100, DY + 80, 130, 12)) field = 2;
            } else if (page == P_DATETIME) {
                for (int i = 0; i < 5; i++) {
                    int y = DY + 40 + i * 18;
                    if (ui_in(e.x, e.y, DX + 136, y, 16, 12)) adj(i, -1);
                    if (ui_in(e.x, e.y, DX + 154, y, 16, 12)) adj(i, 1);
                }
            } else if (page == P_PERSONAL) {
                if (ui_in(e.x, e.y, DX + 10, DY + 52, 240, 12)) { cfg.wall_color = (e.x - DX - 10) / 15; wallpaper_rebuild(); }
                else if (ui_in(e.x, e.y, DX + 110, DY + 78, 16, 12)) { cfg.wall_pattern = (cfg.wall_pattern + NUM_PATTERNS - 1) % NUM_PATTERNS; wallpaper_rebuild(); }
                else if (ui_in(e.x, e.y, DX + 128, DY + 78, 16, 12)) { cfg.wall_pattern = (cfg.wall_pattern + 1) % NUM_PATTERNS; wallpaper_rebuild(); }
                else if (ui_in(e.x, e.y, DX + 110, DY + 100, 16, 12)) cfg.theme = (cfg.theme + NUM_THEMES - 1) % NUM_THEMES;
                else if (ui_in(e.x, e.y, DX + 128, DY + 100, 16, 12)) cfg.theme = (cfg.theme + 1) % NUM_THEMES;
            }
        }
        while ((k = kbd_get()) >= 0) {
            if (page == P_ADMIN) {
                if (k == KEY_ENTER) next();
                else if (k == KEY_BS) { if (al) apass[--al] = 0; }
                else if (k >= 32 && k < 127 && al < 15) { apass[al++] = k; apass[al] = 0; }
            } else if (page == P_ACCOUNT) {
                if (k == KEY_TAB) field = field == 1 ? 2 : 1;
                else if (k == KEY_ENTER) { if (field == 1) field = 2; else next(); }
                else if (k == KEY_BS) { if (field == 1 && ul) user[--ul] = 0; else if (field == 2 && pl) pass[--pl] = 0; }
                else if (k >= 32 && k < 127) {
                    if (field == 1 && ul < 15) { user[ul++] = k; user[ul] = 0; }
                    else if (field == 2 && pl < 15) { pass[pl++] = k; pass[pl] = 0; }
                }
            } else if (k == KEY_ENTER && page != P_INSTALL) next();
        }
        draw();
        sleep_ticks(2);
    }
}

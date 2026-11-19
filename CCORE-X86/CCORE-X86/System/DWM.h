/* CCORE v0.2 - DWM.h : system headers, macros & interface structures */
#ifndef DWM_H
#define DWM_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef short s16;
typedef int s32;
#define NULL ((void *)0)

#define OS_NAME "CCORE"
#define OS_VERSION "0.2"

extern int scr_w, scr_h;
#define SCR_W scr_w
#define SCR_H scr_h
#define TASKBAR_H 15
#define DESK_H (SCR_H - TASKBAR_H)
#define BORDER 3
#define TITLE_H 10
#define CLI_X BORDER
#define CLI_Y (BORDER + TITLE_H + 1)
#define MAX_WINDOWS 16
#define MAX_FILES 64
#define FILE_SECT 8
#define FILE_MAXB (FILE_SECT * 512)
#define NAME_MAX_LEN 19

enum { C_BLACK, C_BLUE, C_GREEN, C_CYAN, C_RED, C_MAGENTA, C_BROWN, C_LGRAY,
       C_DGRAY, C_LBLUE, C_LGREEN, C_LCYAN, C_LRED, C_LMAGENTA, C_YELLOW, C_WHITE };

enum { APP_NONE, APP_CALC, APP_CVER, APP_FILEMGR, APP_NOTEPAD, APP_PAINT,
       APP_SETTINGS, APP_TASKMAN, APP_MSG, APP_CMD };

#define KEY_ESC 27
#define KEY_BS 8
#define KEY_TAB 9
#define KEY_ENTER 10
#define KEY_LEFT 0x101
#define KEY_RIGHT 0x102
#define KEY_UP 0x103
#define KEY_DOWN 0x104
#define KEY_HOME 0x105
#define KEY_END 0x106
#define KEY_DEL 0x107
#define KEY_PGUP 0x108
#define KEY_PGDN 0x109

#define MOUSE_DOWN 1
#define MOUSE_MOVE 2
#define MOUSE_UP 3
#define BTN_L 1
#define BTN_R 2

typedef struct { u32 edi, esi, ebp, esp_d, ebx, edx, ecx, eax, int_no, err_code, eip, cs, eflags; } Regs;
typedef struct { int type, x, y, btn; } MouseEvt;

typedef struct Window Window;
struct Window {
    int used, pid, app;
    int x, y, cw, ch;                 /* outer position, client size */
    int rx, ry, rcw, rch;             /* restore geometry */
    int minimized, maximized;
    char title[32];
    u8 bg;
    u32 start_tick, mem_kb;
    void (*draw)(Window *);
    void (*mouse)(Window *, int type, int x, int y, int btn);  /* client coords */
    void (*key)(Window *, int key);
    void (*tick)(Window *);
    void (*onclose)(Window *);
};

typedef struct { u8 wall_color, wall_pattern, theme, trails, trail_len, mouse_speed, res, reserved; } Settings;
typedef struct { char name[20]; u32 size; u32 lba; u8 used; u8 pad[3]; } __attribute__((packed)) DirEntry;

#define DE_SYS 1                     /* DirEntry.pad[0] flag: system file (admin only) */
#define NUM_THEMES 5
#define NUM_PATTERNS 5
extern Settings cfg;
extern const char *const theme_names[NUM_THEMES];
extern const u8 theme_active[NUM_THEMES], theme_text[NUM_THEMES];
extern const char *const pattern_names[NUM_PATTERNS];

extern volatile u32 sys_ticks;
extern u32 ram_kb;
extern char cpu_name[49];
extern int cpu_ok, has_user, mouse_found, mouse_x, mouse_y;
extern volatile int mouse_btn;
extern char cur_user[16], acct_user[16], acct_pass[16], admin_pass[16];
extern int cur_admin, cur_res, vbe_ok;
extern const int res_w[3], res_h[3];
extern const char *const res_names[3];
extern Window wins[MAX_WINDOWS];
extern int zorder[MAX_WINDOWS], zcount;
extern int ata_ok, fs_ok;
extern u32 ata_sectors;
extern char ata_model[41];
extern DirEntry fs_dir[MAX_FILES];
extern u8 __bss_end[];

/* ports */
static inline void outb(u16 p, u8 v) { __asm__ volatile("outb %0,%1" : : "a"(v), "Nd"(p)); }
static inline u8 inb(u16 p) { u8 v; __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(p)); return v; }
static inline void outw(u16 p, u16 v) { __asm__ volatile("outw %0,%1" : : "a"(v), "Nd"(p)); }
static inline void outl(u16 p, u32 v) { __asm__ volatile("outl %0,%1" : : "a"(v), "Nd"(p)); }
static inline u32 inl(u16 p) { u32 v; __asm__ volatile("inl %1,%0" : "=a"(v) : "Nd"(p)); return v; }
static inline u16 inw(u16 p) { u16 v; __asm__ volatile("inw %1,%0" : "=a"(v) : "Nd"(p)); return v; }
static inline int ui_in(int px, int py, int x, int y, int w, int h) { return px >= x && px < x + w && py >= y && py < y + h; }
static inline u32 app_mem_kb(u32 bytes) { return 4 + (bytes + 1023) / 1024; }

/* libc-lite */
void *memset(void *d, int c, u32 n);
void *memcpy(void *d, const void *s, u32 n);
void *memmove(void *d, const void *s, u32 n);
u32 strlen(const char *s);
int strcmp(const char *a, const char *b);
char *strcpy(char *d, const char *s);
void str_ncpy(char *d, const char *s, int n);
int str_eq_nocase(const char *a, const char *b);
char *fmt_u(char *d, u32 v);
char *fmt_hex(char *d, u32 v, int digits);
char *fmt_2d(char *d, u32 v);
char *fmt_str(char *d, const char *s);
void fmt_hms(char *d, u32 secs);

/* graphics (coordinates are relative to the current origin) */
void gfx_set_origin(int x, int y);
void gfx_set_clip(int x0, int y0, int x1, int y1);
void gfx_pixel(int x, int y, u8 c);
void gfx_rect(int x, int y, int w, int h, u8 c);
void gfx_hline(int x, int y, int w, u8 c);
void gfx_vline(int x, int y, int h, u8 c);
void gfx_frame(int x, int y, int w, int h, u8 c);
void gfx_bevel(int x, int y, int w, int h, int raised);
void gfx_sunken(int x, int y, int w, int h, u8 bg);
void gfx_button(int x, int y, int w, int h, const char *label, int pressed);
void gfx_char(int x, int y, int ch, u8 c);
void gfx_text(int x, int y, const char *s, u8 c);
void gfx_text_n(int x, int y, const char *s, u8 c, int n);
void gfx_text_big(int x, int y, const char *s, u8 c, int scale);
void gfx_blit(int x, int y, int w, int h, const u8 *src);
void gfx_vscroll(int x, int y, int w, int h, int pos, int total, int visible);
void gfx_flip(void);

/* windows */
Window *win_create(const char *title, int app, int x, int y, int cw, int ch, u32 mem_kb);
Window *win_find(int app);
Window *win_by_pid(int pid);
void win_close(Window *w);
void win_focus(Window *w);
Window *win_top(void);
void dwm_message(const char *text);

/* system */
u32 uptime_secs(void);
void rtc_read(int *h, int *m, int *s);
void rtc_date(int *y, int *mo, int *d);
int kbd_get(void);
int mouse_next(MouseEvt *e);
void draw_mouse(void);
void gfx_reset(void);
void gfx_wallpaper(void);
void icons_arrange(void);
int fs_format(const char *admin_pw, const char *user, const char *pass);
int fs_is_sys(int idx);
int fs_rename(int idx, const char *newname);
int fs_set_sys(int idx, int on);
int fs_copy(int src, const char *dst);
void setup_run(void);
void sleep_ticks(u32 n);
void sys_reboot(void);
void sys_shutdown(void);
void bsod_manual(void);
void bsod_stop(const char *name, const char *detail);
int dwm_set_resolution(int r);
void vbe_init(void);
u32 gfx_fb_addr(void);
u32 read_cr0(void);
void wallpaper_rebuild(void);
void settings_save(void);
void settings_apply(void);

/* disk / file system */
int ata_read(u32 lba, u32 count, void *buf);
int ata_write(u32 lba, u32 count, const void *buf);
int fs_find(const char *name);
int fs_create(const char *name);
int fs_delete(int idx);
int fs_read(int idx, void *buf, u32 max);
int fs_write(int idx, const void *buf, u32 size);
int fs_rescan(void);

/* apps */
void calc_launch(void);
void cver_launch(void);
void filemgr_launch(void);
void notepad_launch(void);
void notepad_open_file(const char *name);
void paint_launch(void);
void settings_launch(void);
void taskman_launch(void);
void cmd_launch(int admin);

#endif

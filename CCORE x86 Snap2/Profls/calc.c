#include "../System/DWM.h"
extern Window windows[8];
extern int window_count;
extern MouseState mouse;

static char calc_display[16] = "0";
static int calc_stored_val = 0;
static char calc_op = 0;
static int calc_clear_next = 0;

static void int_to_str(int num, char* str) {
    if (num == 0) { str[0] = '0'; str[1] = '\0'; return; }
    int i = 0, is_neg = 0;
    if (num < 0) { is_neg = 1; num = -num; }
    char temp[16];
    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    int j = 0;
    if (is_neg) str[j++] = '-';
    while (i > 0) str[j++] = temp[--i];
    str[j] = '\0';
}

static int str_to_int(const char* str) {
    int res = 0, sign = 1, i = 0;
    if (str[0] == '-') { sign = -1; i = 1; }
    while (str[i] != '\0') {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res * sign;
}

void render_calc_content(Window* self) {
    draw_3d_box(self->x + 10, self->y + 24, self->w - 20, 22, 1);
    draw_string(self->x + 18, self->y + 31, calc_display, 0x00);

    const char* b_labels[] = {
        "7", "8", "9", "+",
        "4", "5", "6", "-",
        "1", "2", "3", "C",
        "0", "="
    };

    int start_x = self->x + 12;
    int start_y = self->y + 54;

    for (int i = 0; i < 14; i++) {
        int row = i / 4;
        int col = i % 4;
        int bx = start_x + col * 32;
        int by = start_y + row * 24;

        int pressed = (mouse.left_btn && mouse.x >= bx && mouse.x <= bx + 26 && mouse.y >= by && mouse.y <= by + 20);
        draw_3d_box(bx, by, 26, 20, pressed ? 1 : 0);

        int offset = pressed ? 1 : 0;
        draw_string(bx + 9 + offset, by + 6 + offset, b_labels[i], 0x00);
    }
}

void click_calc_content(Window* self, int mx, int my) {
    int start_x = self->x + 12;
    int start_y = self->y + 54;

    const char* b_labels[] = {
        "7", "8", "9", "+",
        "4", "5", "6", "-",
        "1", "2", "3", "C",
        "0", "="
    };

    for (int i = 0; i < 14; i++) {
        int row = i / 4;
        int col = i % 4;
        int bx = start_x + col * 32;
        int by = start_y + row * 24;

        if (mx >= bx && mx <= bx + 26 && my >= by && my <= by + 20) {
            char key = b_labels[i][0];

            if (key >= '0' && key <= '9') {
                if (calc_clear_next || (calc_display[0] == '0' && calc_display[1] == '\0')) {
                    calc_display[0] = key;
                    calc_display[1] = '\0';
                    calc_clear_next = 0;
                } else {
                    int len = 0;
                    while (calc_display[len]) len++;
                    if (len < 12) {
                        calc_display[len] = key;
                        calc_display[len + 1] = '\0';
                    }
                }
            } else if (key == '+' || key == '-' || key == '*') {
                calc_stored_val = str_to_int(calc_display);
                calc_op = key;
                calc_clear_next = 1;
            } else if (key == '=') {
                int curr = str_to_int(calc_display);
                int res = curr;
                if (calc_op == '+') res = calc_stored_val + curr;
                if (calc_op == '-') res = calc_stored_val - curr;
                if (calc_op == '*') res = calc_stored_val * curr;
                int_to_str(res, calc_display);
                calc_op = 0;
                calc_clear_next = 1;
            } else if (key == 'C') {
                calc_display[0] = '0';
                calc_display[1] = '\0';
                calc_stored_val = 0;
                calc_op = 0;
                calc_clear_next = 0;
            }
            break;
        }
    }
}

void launch_calc_app(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].title[0] == 'C' && windows[i].title[1] == 'A') {
            windows[i].is_visible = 1;
            return;
        }
    }

    if (window_count < 8) {
        Window* w = &windows[window_count++];
        w->id = 1; w->x = 80; w->y = 50; w->w = 150; w->h = 160;
        w->title[0] = 'C'; w->title[1] = 'A'; w->title[2] = 'L'; w->title[3] = 'C'; w->title[4] = '\0';
        w->is_visible = 1;
        w->render_content = render_calc_content;
        w->click_event = click_calc_content;
    }
}
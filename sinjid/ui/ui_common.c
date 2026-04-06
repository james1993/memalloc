#include "ui_internal.h"

/* ── Font ── */
static Font g_font;
static bool g_font_loaded = false;

void ui_init(void)
{
    const char *ttf = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
    if (FileExists(ttf)) {
        g_font = LoadFontEx(ttf, 20, NULL, 0);
        g_font_loaded = true;
    }
}

void ui_close(void)
{
    if (g_font_loaded) UnloadFont(g_font);
}

/* ── Palette ── */
const Color C_PANEL   = {  28,  28,  45, 255 };
const Color C_BORDER  = {  70,  70, 120, 255 };
const Color C_GOLD    = { 220, 180,  40, 255 };
const Color C_WHITE   = { 230, 230, 230, 255 };
const Color C_DIM     = { 100, 100, 130, 255 };
const Color C_BTN     = {  45,  45,  80, 255 };
const Color C_BTN_HOV = {  70,  70, 130, 255 };
const Color C_BTN_DIS = {  30,  30,  45, 255 };
const Color C_HP      = { 200,  40,  40, 255 };
const Color C_MP      = {  40,  80, 200, 255 };
const Color C_XP      = {  40, 180,  80, 255 };
const Color C_GREEN   = {  60, 200,  60, 255 };
const Color C_RED     = { 220,  60,  60, 255 };

/* ── Font helpers ── */
void txt(int x, int y, int size, Color col, const char *s)
{
    if (g_font_loaded)
        DrawTextEx(g_font, s, (Vector2){(float)x, (float)y},
                   (float)size, 1.0f, col);
    else
        DrawText(s, x, y, size, col);
}

int txt_w(const char *s, int size)
{
    if (g_font_loaded)
        return (int)MeasureTextEx(g_font, s, (float)size, 1.0f).x;
    return MeasureText(s, size);
}

void txt_c(Rectangle r, int size, Color col, const char *s)
{
    int tw = txt_w(s, size);
    int tx = (int)(r.x + (r.width  - tw)   / 2);
    int ty = (int)(r.y + (r.height - size)  / 2);
    txt(tx, ty, size, col, s);
}

/* ── Shared UI widgets ── */
void ui_panel(Rectangle r, Color fill, Color border)
{
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 1.5f, border);
}

void ui_bar(int x, int y, int w, int h, int cur, int max, Color fill, Color bg)
{
    DrawRectangle(x, y, w, h, bg);
    if (max > 0) {
        int filled = (int)((float)cur / (float)max * w);
        if (filled > w) filled = w;
        if (filled > 0) DrawRectangle(x, y, filled, h, fill);
    }
    DrawRectangleLines(x, y, w, h, C_BORDER);
}

void ui_bar_f(int x, int y, int w, int h,
              float cur, int max, Color fill, Color bg)
{
    DrawRectangle(x, y, w, h, bg);
    if (max > 0 && cur > 0) {
        int filled = (int)(cur / (float)max * w);
        if (filled > w) filled = w;
        if (filled > 0) DrawRectangle(x, y, filled, h, fill);
    }
    DrawRectangleLines(x, y, w, h, C_BORDER);
}

void ui_text_center(Rectangle r, const char *text, int font_size, Color col)
{
    txt_c(r, font_size, col, text);
}

bool ui_button(Rectangle r, const char *label, bool enabled)
{
    Vector2 mouse = game_mouse();
    bool hovered  = enabled && CheckCollisionPointRec(mouse, r);
    Color bg      = !enabled ? C_BTN_DIS : (hovered ? C_BTN_HOV : C_BTN);
    Color tc      = !enabled ? C_DIM     : C_WHITE;
    ui_panel(r, bg, C_BORDER);
    txt_c(r, 18, tc, label);

    static Rectangle last_hovered = {0};
    if (hovered && (last_hovered.x != r.x || last_hovered.y != r.y)) {
        snd_hover();
        last_hovered = r;
    }
    if (!hovered && last_hovered.x == r.x && last_hovered.y == r.y)
        last_hovered = (Rectangle){0};

    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (clicked) snd_click();
    return clicked;
}

#pragma once
#include "game.h"

// ── Draw helpers ──────────────────────────────────────────────────────────
// Returns true if mouse clicked inside the rectangle this frame
bool ui_button(Rectangle r, const char *label, bool enabled);

// Draw a filled bar (e.g. HP bar)
void ui_bar(int x, int y, int w, int h, int cur, int max, Color fill, Color bg);

// Draw a panel (filled rounded rect + border)
void ui_panel(Rectangle r, Color fill, Color border);

// Draw text centered inside a rectangle
void ui_text_center(Rectangle r, const char *text, int font_size, Color col);

// ── Scene renderers ───────────────────────────────────────────────────────
void ui_draw_title(void);
void ui_draw_class_select(void);
void ui_draw_hub(void);
void ui_draw_combat(void);
void ui_draw_inventory(void);
void ui_draw_shop(void);
void ui_draw_skills(void);
void ui_draw_level_up(void);
void ui_draw_gateway_select(void);
void ui_draw_game_over(void);
void ui_draw_victory(void);

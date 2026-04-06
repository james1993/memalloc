#pragma once
/* Shared utilities for all ui/ scene files.
   Include this (and only this) at the top of every ui/ui_*.c */

#include <raylib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../game.h"
#include "../scene.h"
#include "../player.h"
#include "../combat.h"
#include "../hub.h"
#include "../data.h"
#include "../anim.h"
#include "../audio.h"
#include "../save.h"
#include "../ui.h"
#include "../events.h"

/* ── Palette (defined in ui_common.c) ───────────────────────────── */
extern const Color C_PANEL;
extern const Color C_BORDER;
extern const Color C_GOLD;
extern const Color C_WHITE;
extern const Color C_DIM;
extern const Color C_BTN;
extern const Color C_BTN_HOV;
extern const Color C_BTN_DIS;
extern const Color C_HP;
extern const Color C_MP;
extern const Color C_XP;
extern const Color C_GREEN;
extern const Color C_RED;

/* ── Font helpers (defined in ui_common.c) ──────────────────────── */
void txt  (int x, int y, int size, Color col, const char *s);
int  txt_w(const char *s, int size);
void txt_c(Rectangle r, int size, Color col, const char *s);

/* Mouse position scaled from window space → game canvas (SCREEN_W × SCREEN_H).
   Always use this instead of GetMousePosition() so input works correctly when
   the window is a different size from the internal render resolution. */
static inline Vector2 game_mouse(void)
{
    Vector2 m = GetMousePosition();
    m.x = m.x * (float)SCREEN_W / (float)GetScreenWidth();
    m.y = m.y * (float)SCREEN_H / (float)GetScreenHeight();
    return m;
}

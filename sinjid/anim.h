#pragma once
#include <raylib.h>
#include <stdbool.h>

/* ── Per-combat animated bar values ── */
typedef struct {
    float player_life;   // display value, lerps toward actual
    float player_mana;
    float enemy_life;
    float enemy_mana;

    float player_flash;  // 0..1, fades to 0 (white hit flash)
    float enemy_flash;
    bool  player_flash_col; // false=white(hit), true=green(heal)
    bool  enemy_flash_col;

    float player_bob_t;  // idle bob time accumulator (player card)
    float enemy_bob_t;
} CombatAnim;

/* ── Screen-level animation ── */
typedef struct {
    float     fade_alpha;   // 0=clear 1=black
    bool      fading_out;   // true: going clear→black, false: black→clear
    bool      active;
    int       target_scene; // switch g_scene when fade_alpha hits 1
} FadeAnim;

/* ── Global anim state ── */
extern CombatAnim g_canim;
extern FadeAnim   g_fade;
extern bool       g_scene_dirty; // static layer needs redraw

/* Call once per frame before drawing */
void anim_update(float dt);

/* Trigger a scene fade; switches scene at the midpoint */
void anim_fade_to(int scene);

/* Trigger hit flash on player or enemy */
void anim_player_hit(bool heal);
void anim_enemy_hit(bool heal);

/* Reset combat anim to match current player/enemy state */
void anim_combat_reset(void);

/* Y offset for idle bob (pass player_bob_t or enemy_bob_t) */
float anim_bob_y(float t);
